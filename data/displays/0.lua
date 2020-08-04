local battPowerKW = (CANServer_getVar("BattVolts") * CANServer_getVar("BattAmps") / 1000.0)
local graphBattPower = math.floor(math.min(math.max((24) * (battPowerKW) / (300), -24), 24))

return "65535c" .. math.floor(battPowerKW * 10) .. "vWK  Bu" .. graphBattPower .. "b0m100r"
