function setSpace(newSpace)
    if newSpace == nil then
        engine:resetSpace()
        print("entered global space")
    else
        engine:setSpace(newSpace)
        print("entered space: " .. newSpace)
    end
end
