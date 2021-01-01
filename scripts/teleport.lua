require "plugins\\cyber_engine_tweaks\\scripts\\math\\math-missing"
require "plugins\\cyber_engine_tweaks\\scripts\\math\\math"
require "plugins\\cyber_engine_tweaks\\scripts\\math\\vector3"
require "plugins\\cyber_engine_tweaks\\scripts\\math\\quaternion"

-- Quaternion to Euler
function getPlayerRotation(orientation)
    quat = Quaternion.New(orientation.i, orientation.j, orientation.k, orientation.r)
    euler = quat:ToEulerAngles()
    return EulerAngles.new(euler.x,euler.y,euler.z)
end

-- Get Vector4 "x paces" from player forward pos
function getTeleportDestination(player, distance)
    pos = player:GetWorldPosition()
    forward = player:GetWorldForward()
    return Vector4.new(pos.x + (forward.x * distance), pos.y + (forward.y * distance), pos.z + forward.z, pos.w + forward.w)
end

function teleportForward(distance)
    player = Game.GetPlayer()
    rot = getPlayerRotation(player:GetWorldOrientation())
    dest = getTeleportDestination(player, distance)
    tp = Game.GetTeleportationFacility()
    tp:Teleport(player, dest, rot)
end