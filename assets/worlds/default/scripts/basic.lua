function setSpace(newSpace)
    if newSpace == nil then
        engine:resetSpace()
    else
        engine:setSpace(newSpace)
    end
end

function enterHouse()
    local outsideTileProxies = engine:getTileGroupProxies(0)
    local insideTileProxies = engine:getTileGroupProxies(1)
    local wallTileProxies = engine:getTileGroupProxies(2)

    local tiles = engine:getTileInstances()

    for i = 1, #outsideTileProxies do
        local proxy = outsideTileProxies[i]
        local tile = tiles[proxy.index]

        tile.colourMultiplier.r = 0.0
        tile.colourMultiplier.g = 0.0
        tile.colourMultiplier.b = 0.0
    end

    for i = 1, #insideTileProxies do
        local proxy = insideTileProxies[i]
        local tile = tiles[proxy.index]

        tile.colourMultiplier.r = 1.0
        tile.colourMultiplier.g = 1.0
        tile.colourMultiplier.b = 1.0
    end

    for i = 1, #wallTileProxies do
        local proxy = wallTileProxies[i]
        local tile = tiles[proxy.index]

        tile.colourMultiplier.r = 0.5
        tile.colourMultiplier.g = 0.5
        tile.colourMultiplier.b = 0.5
    end
end

function leaveHouse()
    local outsideTileProxies = engine:getTileGroupProxies(0)
    local insideTileProxies = engine:getTileGroupProxies(1)
    local wallTileProxies = engine:getTileGroupProxies(2)

    local tiles = engine:getTileInstances()

    for i = 1, #outsideTileProxies do
        local proxy = outsideTileProxies[i]
        local tile = tiles[proxy.index]

        tile.colourMultiplier.r = 1.0
        tile.colourMultiplier.g = 1.0
        tile.colourMultiplier.b = 1.0
    end

    for i = 1, #insideTileProxies do
        local proxy = insideTileProxies[i]
        local tile = tiles[proxy.index]

        tile.colourMultiplier.r = 0.0
        tile.colourMultiplier.g = 0.0
        tile.colourMultiplier.b = 0.0
    end

    for i = 1, #wallTileProxies do
        local proxy = wallTileProxies[i]
        local tile = tiles[proxy.index]

        tile.colourMultiplier.r = 1.0
        tile.colourMultiplier.g = 1.0
        tile.colourMultiplier.b = 1.0
    end
end
