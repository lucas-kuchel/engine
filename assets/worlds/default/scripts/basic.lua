function setSpace(engine, newSpace)
    if newSpace == nil then
        engine:resetSpace()
    else
        engine:setSpace(newSpace)
    end
end
