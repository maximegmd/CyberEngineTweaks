function main (target)
	local host = os.host()
	local subhost = os.subhost()

	local system
	if (host ~= subhost) then
		system = host .. "/" .. subhost
	else
		system = host
	end

	local branch = "unknown-branch"
	local commitHash = "unknown-commit"
	try
	{
		function ()
			import("detect.tools.find_git")
			local git = find_git()
			if (git) then
				branch = os.iorunv(git, {"rev-parse", "--abbrev-ref", "HEAD"}):trim()
				commitHash = os.iorunv(git, {"describe", "--tags"}):trim()
			else
				error("git not found")
			end
		end,

		catch
		{
			function (err)
				print(string.format("Failed to retrieve git data: %s", err))
			end
		}
    }

    return branch, commitHash
end