function setSpace(newSpace)
    engine:setSpace(newSpace)

    local playerProxies = engine:getTileGroupProxies(5)

    for i = 1, #playerProxies do
        local proxy = playerProxies[i]

        engine:addToGroup(proxy, 1)
    end
end

function resetSpace()
    engine:resetSpace()

    local playerProxies = engine:getTileGroupProxies(5)

    for i = 1, #playerProxies do
        local proxy = playerProxies[i]

        engine:removeFromGroup(proxy, 1)
    end
end

function openDoor()
    local doorProxies = engine:getTileGroupProxies(3)
    local tiles = engine:getTileInstances()

    for i = 1, #doorProxies do
        local proxy = doorProxies[i]
        local tile = tiles[proxy.index]

        tile.transform.scale.x = 0.25
        tile.appearance.texture.repeats.x = 0.25
    end
end

function closeDoor()
    local doorProxies = engine:getTileGroupProxies(3)
    local tiles = engine:getTileInstances()

    for i = 1, #doorProxies do
        local proxy = doorProxies[i]
        local tile = tiles[proxy.index]

        tile.transform.scale.x = 1.0
        tile.appearance.texture.repeats.x = 1.0
    end
end
