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
    fadeTileProxies(wallTileProxies, tiles, 0.5, t)
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
    fadeTileProxies(insideTileProxies, tiles, 0.0, t)
    fadeTileProxies(wallTileProxies, tiles, 1.0, t)
end
