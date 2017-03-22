function normalFunc(data)
    if (data:find("EnterStealth",1,true)) then
        print("[-->Stealth Mode]<--");
        thisRec=nil;
        uart.on("data",0,stealthFunc,0);
    end
end

function stealthFunc(data)
    if (data:find("ExitStealth",1,true)) then
        print("-->[Normal Mode]<--");
        uart.on("data",0,normalFunc,1);
    elseif (data=="[\r\n") then
        file.remove("data.json");
        file.open("data.json","w+");
        file.write("[");
        file.close();
        thisRec=0;
    elseif (data=="]\r\n") then
        file.open("data.json","a");
        file.writeline("\r\n]");
        file.close();
        thisRec=nil;
    elseif (thisRec~=nil) then
        local JSONString="\r\n";
        if (thisRec~=0) then
            JSONString=",\r\n{"..JSONString;
        else
            JSONString="\r\n{"..JSONString;
        end
        thisRec=thisRec+1;
        for k,v in string.gmatch(data,"(%w+):([%w+%s/_=|<>]+),*") do
            JSONString=JSONString.."\""..k.."\":\""..v.."\","
        end
        JSONString=JSONString:sub(1,#JSONString-1);
        JSONString=JSONString.."}"
        file.open("data.json","a");
        file.write(JSONString);
        file.close();
    else
        print(data);
    end
end
