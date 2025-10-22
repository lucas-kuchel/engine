function setSpace(newSpace)
    if newSpace == nil then
        engine:resetSpace()
        print("entered global space")
    else
        engine:setSpace(newSpace)
        print("entered space: " .. newSpace)
    end
end

function changeTile()
    local ok, err = pcall(function()
        print("changing tile")

        local tileProxies = engine:getTileGroupProxies(0)
        print("group size:", #tileProxies)

        if #tileProxies == 0 then return end

        for i, proxy in ipairs(tileProxies) do
            print("proxy at", i, proxy, proxy.index)
        end

        local firstProxy = tileProxies[1]
        print("firstProxy.index:", firstProxy.index)

        local tiles = engine:getTiles()
        print("tiles count:", #tiles)

        -- choose whether tiles are 1-based or 0-based in your binding:
        local idx = firstProxy.index + 1 -- use +1 if tiles[] is 1-based
        print("attempting index:", idx)

        if idx < 1 or idx > #tiles then
            print("index out of range, aborting")
            return
        end

        -- wrap the nested member modification in another pcall
        local ok2, err2 = pcall(function()
            local tileInstance = tiles[idx]
            tileInstance.texture.offset.x = tileInstance.texture.offset.x + 0.1
            tileInstance.texture.offset.y = tileInstance.texture.offset.y + 0.2
        end)

        if not ok2 then
            print("failed modifying tileInstance:", err2)
        end
    end)

    if not ok then
        print("changeTile failed:", err)
    end
end
