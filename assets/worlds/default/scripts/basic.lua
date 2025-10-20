function setSpace(newSpace)
    print("lua script was run!")
    if newSpace == nil then
        print("space reset")
        engine:resetSpace()
    else
        print("space set")
        engine:setSpace(newSpace)
    end

    print("script reached end")
end
