function setSpace(newSpace)
    if newSpace == nil then
        engine:resetSpace()
    else
        engine:setSpace(newSpace)
    end
end

function darkenOutside()
    local tileProxies = engine:getTileGroupProxies(0)

    if #tileProxies == 0 then return end

    local tiles = engine:getTileInstances()

    for i = 1, #tileProxies do
        local proxy = tileProxies[i]

        if proxy.index <= #tiles then
            local tile = tiles[proxy.index]

            tile.colourMultiplier.r = 0.1
            tile.colourMultiplier.g = 0.1
            tile.colourMultiplier.b = 0.1
        end
    end
end

function brightenOutside()
    local tileProxies = engine:getTileGroupProxies(0)

    if #tileProxies == 0 then return end

    local tiles = engine:getTileInstances()

    for i = 1, #tileProxies do
        local proxy = tileProxies[i]

        if proxy.index <= #tiles then
            local tile = tiles[proxy.index]

            tile.colourMultiplier.r = 1.0
            tile.colourMultiplier.g = 1.0
            tile.colourMultiplier.b = 1.0
        end
    end
end
