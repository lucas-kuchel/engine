function setSpace(newSpace)
    if newSpace == nil then
        engine:resetSpace()
    else
        engine:setSpace(newSpace)
    end
end

function lerp(a, b, t)
    return a + (b - a) * t
end

function fadeTileProxies(proxies, tiles, finalFactor, t)
    for i = 1, #proxies do
        local proxy = proxies[i]
        local tile = tiles[proxy.index]

        tile.colourMultiplier.r = lerp(tile.colourMultiplier.r, finalFactor, t)
        tile.colourMultiplier.g = lerp(tile.colourMultiplier.g, finalFactor, t)
        tile.colourMultiplier.b = lerp(tile.colourMultiplier.b, finalFactor, t)
    end
end

function enterHouse()
    local outsideTileProxies = engine:getTileGroupProxies(0)
    local insideTileProxies = engine:getTileGroupProxies(1)
    local wallTileProxies = engine:getTileGroupProxies(2)
    local tiles = engine:getTileInstances()
    local elapsed = engine:getActionTimeElapsed()
    local duration = engine:getActionDuration()

    local t = math.min(elapsed / duration, 1.0)

    fadeTileProxies(outsideTileProxies, tiles, 0.0, t)
    fadeTileProxies(insideTileProxies, tiles, 1.0, t)
    fadeTileProxies(wallTileProxies, tiles, 0.25, t)
end

function leaveHouse()
    local outsideTileProxies = engine:getTileGroupProxies(0)
    local insideTileProxies = engine:getTileGroupProxies(1)
    local wallTileProxies = engine:getTileGroupProxies(2)
    local tiles = engine:getTileInstances()
    local elapsed = engine:getActionTimeElapsed()
    local duration = engine:getActionDuration()

    local t = math.min(elapsed / duration, 1.0)

    fadeTileProxies(outsideTileProxies, tiles, 1.0, t)
    fadeTileProxies(insideTileProxies, tiles, 0.25, t)
    fadeTileProxies(wallTileProxies, tiles, 1.0, t)
end

function openDoor()
    local doorProxies = engine:getTileGroupProxies(3)
    local indoorProxies = engine:getTileGroupProxies(1)
    local tiles = engine:getTileInstances()

    for i = 1, #doorProxies do
        local proxy = doorProxies[i]
        local tile = tiles[proxy.index]

        tile.texture.sample.position.x = 0.2
    end

    for i = 1, #indoorProxies do
        local proxy = indoorProxies[i]
        local tile = tiles[proxy.index]

        tile.colourMultiplier.r = 1.0
        tile.colourMultiplier.g = 1.0
        tile.colourMultiplier.b = 1.0
    end
end

function closeDoor()
    local doorProxies = engine:getTileGroupProxies(3)
    local indoorProxies = engine:getTileGroupProxies(1)
    local tiles = engine:getTileInstances()

    for i = 1, #doorProxies do
        local proxy = doorProxies[i]
        local tile = tiles[proxy.index]

        tile.texture.sample.position.x = 0.0
    end

    for i = 1, #indoorProxies do
        local proxy = indoorProxies[i]
        local tile = tiles[proxy.index]

        tile.colourMultiplier.r = 0.25
        tile.colourMultiplier.g = 0.25
        tile.colourMultiplier.b = 0.25
    end
end
