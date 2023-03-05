print('Hello GMP')

addEventHandler('OnClockUpdate', function(day, hour, minute)
    print("Server clock has been updated. Day " .. day .. " Hour " .. hour .. " Minute " .. minute .. ".")
end)

addEventHandler('OnPlayerConnect', function(id)
    print("Player " .. id .. " has connected.")
end)