#include "stdafx.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.h"

#include "CET.h"

void Texture::BindTexture(sol::table& aTable)
{
    aTable.new_usertype<Texture>("ImguiTexture", sol::no_constructor, "size", sol::property(&Texture::GetSize), "Release", &Texture::Release);

    aTable.set_function(
        "Image",
        sol::overload(
            &Texture::ImGuiImage,
            [](const Texture& acTexture, ImVec2 aSize, const ImVec2& aUv0, const ImVec2& aUv1, const ImVec4& aTintCol) { ImGuiImage(acTexture, aSize, aUv0, aUv1, aTintCol); },
            [](const Texture& acTexture, ImVec2 aSize, const ImVec2& aUv0, const ImVec2& aUv1) { ImGuiImage(acTexture, aSize, aUv0, aUv1); },
            [](const Texture& acTexture, ImVec2 aSize, const ImVec2& aUv0) { ImGuiImage(acTexture, aSize, aUv0); },
            [](const Texture& acTexture, ImVec2 aSize) { ImGuiImage(acTexture, aSize); }, [](const Texture& acTexture) { ImGuiImage(acTexture); }));
}

std::shared_ptr<Texture> Texture::Load(const std::string& acPath)
{
    auto d3d_device = CET::Get().GetD3D12().GetDevice();
    if (d3d_device == nullptr)
        return {};

    auto [srvCpuHandle, srvGpuHandle] = CET::Get().GetD3D12().CreateTextureDescriptor();

    if (srvCpuHandle.ptr == 0 || srvGpuHandle.ptr == 0)
    {
        spdlog::error("maximum number of textures reached!");
        return {};
    }

    // Load from disk into a raw RGBA buffer
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(acPath.c_str(), &image_width, &image_height, nullptr, 4);
    if (image_data == nullptr)
        return {};

    // Create texture resource
    D3D12_HEAP_PROPERTIES props = {};
    props.Type = D3D12_HEAP_TYPE_DEFAULT;
    props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = 0;
    desc.Width = image_width;
    desc.Height = image_height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    Microsoft::WRL::ComPtr<ID3D12Resource> pTexture = nullptr;
    d3d_device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&pTexture));

    // Create a temporary upload resource to move the data in
    UINT uploadPitch = (image_width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
    UINT uploadSize = image_height * uploadPitch;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = uploadSize;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    props.Type = D3D12_HEAP_TYPE_UPLOAD;
    props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer = nullptr;
    HRESULT hr = d3d_device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
    IM_ASSERT(SUCCEEDED(hr));

    // Write pixels into the upload resource
    void* mapped = nullptr;
    D3D12_RANGE range = {0, uploadSize};
    hr = uploadBuffer->Map(0, &range, &mapped);
    IM_ASSERT(SUCCEEDED(hr));
    for (int y = 0; y < image_height; y++)
        memcpy(static_cast<char*>(mapped) + y * uploadPitch, image_data + y * image_width * 4, image_width * 4);
    uploadBuffer->Unmap(0, &range);

    // Copy the upload resource content into the real resource
    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = uploadBuffer.Get();
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srcLocation.PlacedFootprint.Footprint.Width = image_width;
    srcLocation.PlacedFootprint.Footprint.Height = image_height;
    srcLocation.PlacedFootprint.Footprint.Depth = 1;
    srcLocation.PlacedFootprint.Footprint.RowPitch = uploadPitch;

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource = pTexture.Get();
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = 0;

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = pTexture.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    // Create a temporary command queue to do the copy with
    Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
    hr = d3d_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    IM_ASSERT(SUCCEEDED(hr));

    HANDLE event = CreateEvent(nullptr, 0, 0, nullptr);
    IM_ASSERT(event != nullptr);

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 1;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> cmdQueue = nullptr;
    hr = d3d_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cmdQueue));
    IM_ASSERT(SUCCEEDED(hr));

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdAlloc = nullptr;
    hr = d3d_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc));
    IM_ASSERT(SUCCEEDED(hr));

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList = nullptr;
    hr = d3d_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&cmdList));
    IM_ASSERT(SUCCEEDED(hr));

    cmdList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
    cmdList->ResourceBarrier(1, &barrier);

    hr = cmdList->Close();
    IM_ASSERT(SUCCEEDED(hr));

    // Execute the copy
    ID3D12CommandList* commandLists[] = {cmdList.Get()};
    cmdQueue->ExecuteCommandLists(1, commandLists);
    hr = cmdQueue->Signal(fence.Get(), 1);
    IM_ASSERT(SUCCEEDED(hr));

    // Wait for everything to complete
    fence->SetEventOnCompletion(1, event);
    WaitForSingleObject(event, INFINITE);

    // Tear down our temporary command queue and release the upload resource
    cmdList->Release();
    cmdAlloc->Release();
    cmdQueue->Release();
    CloseHandle(event);
    fence->Release();
    uploadBuffer->Release();

    // Create a shader resource view for the texture
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    d3d_device->CreateShaderResourceView(pTexture.Get(), &srvDesc, srvCpuHandle);

    // Return results
    auto texture = std::make_shared<Texture>();
    texture->m_size = ImVec2(static_cast<float>(image_width), static_cast<float>(image_height));
    texture->m_texture = pTexture;
    texture->m_handle = srvGpuHandle;

    stbi_image_free(image_data);

    return texture;
}

void Texture::ImGuiImage(const Texture& acTexture, ImVec2 aSize, const ImVec2& aUv0, const ImVec2& aUv1, const ImVec4& aTintCol, const ImVec4& aBorderCol)
{
    if (aSize.x == 0 && aSize.y == 0)
        aSize = acTexture.m_size;

    ImGui::Image(reinterpret_cast<ImTextureID>(acTexture.m_handle.ptr), aSize, aUv0, aUv1, aTintCol, aBorderCol);
}

ImVec2 Texture::GetSize() const
{
    return m_size;
}

void Texture::Release()
{
    m_texture = nullptr;
}
