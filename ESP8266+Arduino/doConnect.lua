return function()
    wifi.setmode(wifi.STATION)
    wifi.sta.setip({ip="192.168.1.100",netmask="255.255.255.0",gateway="192.168.1.1"});
    if file.open("password.txt") then
        local username=file.readline();
        local password=file.readline();
        file.close();
        username=string.sub(username,1,#username-1);
        password=string.sub(password,1,#password-1);
        wifi.sta.config(username,password);
        tmr.alarm(1, 1000, 1, function()
            if wifi.sta.getip() == nil then
                print("IP unavaiable, waiting.")
            else
                tmr.stop(1)
                print("Connected, IP is "..wifi.sta.getip())
            end
        end)        
    else
        print("[cannt open password file]");
    end
end
