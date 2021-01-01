math.deg2Rad = math.pi / 180

function math.atan2(a, b)
	atan2val = 0.0

    if b > 0 then
        atan2val = math.atan(a/b);
    elseif (b < 0) and (a >= 0) then
        atan2val = math.atan(a/b) + math.pi
    elseif (b < 0) and (a < 0) then
        atan2val = math.atan(a/b) - math.pi
    elseif (b == 0) and (a > 0) then
        atan2val = math.pi / 2
    elseif (b == 0) and (a < 0) then
        atan2val = 0 - (p / 2)
    elseif (b == 0) and (a == 0) then
        atan2val = 1000 -- represents undefined
    end

    return atan2val
end