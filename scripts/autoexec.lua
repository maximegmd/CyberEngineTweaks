Game['DebugGiveHotkeys'] = Game['DebugGiveHotkeys;GameInstance']
Game['DebugNPCs_NonExec'] = Game['DebugNPCs_NonExec;GameInstanceStringStringString;GameInstance']
Game['GetPlayer'] = Game['GetPlayer;GameInstance']
Game['GetPlayerObject'] = Game['GetPlayerObject;GameInstance']
Game['PlayFinisher'] = Game['PlayFinisher;GameInstance']
Game['PlayFinisherSingle'] = Game['PlayFinisherSingle;GameInstance']
Game['PPS'] = Game['PPS;GameInstance']

print("Cyber Engine Tweaks autoexec.lua execution complete.")

require "plugins\\cyber_engine_tweaks\\scripts\\teleport"
print("Teleport loaded; Usage 'teleportForward(5)'")