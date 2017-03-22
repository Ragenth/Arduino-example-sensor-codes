local srv=net.createServer(net.TCP)
srv:listen(80,function(conn)
    local ThreadID
    local function prepareFile(c,myFile)
        ThreadID=coroutine.create(function(c,myFile)
        dofile("getPartialData.lc")(c,myFile)        
        end) 
        local status, err=coroutine.resume(ThreadID,c,myFile);
        if not status then
            print("error="..err);
        end
    end
    local function todoOnReceive(c,payload)
        local getData=dofile("getMethod.lc")(payload);
        local Header=nil;
        print("["..getData.file.name.."]");
        if (getData.fileExist=="true") then
            if (getData.file.name=="data.json") then
                c:send("HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: application/json; charset=UTF-8\r\nContent-Length: "..getData.file.size.."\r\n\r\n");
                if (getData.file.value=="fromArduino") then
                    uart.on("data","\n",stealthFunc,0);
                    uart.write(0,"readCMDS},");
                    thisRec=0;
                    local tmpFile={};                    
                    tmpFile=getData.file;
                    tmr.alarm(0,3000,1,function()
                        print("waiting");
                        if (thisRec==nil) then
                            tmr.stop(0);
                            uart.on("data");
                            prepareFile(c,tmpFile);
                            tmpFile=nil;
                        end
                    end)
                else 
                    prepareFile(c,getData.file);
                end
            else
                c:send("HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Length: "..getData.file.size.."\r\n\r\n");
                prepareFile(c,getData.file);
            end
        elseif (getData.file.name=="luaCMD") then
                if (getData.file.value=="ESP_Restart") then
                    c:send("HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Length: "..#getData.file.value.."\r\n\r\n");
                    c:send(getData.file.value);
                    tmr.alarm(0, 3000, 0, function()
                        node.restart();
                    end)
                end
        elseif (getData.file.name=="runCommand") then
                c:send("HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Length: "..getData.file.size.."\r\n\r\n");
                c:send(getData.file.name);
                uart.write(0,"run="..getData.file.value.."},");
        elseif (getData.file.name=="update") then
                local tmp="";
                if (getData.file.start==nil) then
                    tmp="send me data";
                    file.open("data.json","a");
                    file.write(getData.file.value.."\r\n")
                    file.close();         
                    uart.write(0,"rec="..getData.file.value.."\n");
                else
                    tmp="file created\r\n"
                    file.open("data.json","w+");
                    file.write("[\r\n");
                    file.close();
                    if (getData.file.start=="0") then
                        uart.write(0,"delCMDS},");
                        file.open("data.json","a");
                        file.write("]\r\n");
                        file.close();
                    else
                        uart.write(0,"totalRec="..getData.file.start.."},");
                    end
                end
                c:send("HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Length: "..#tmp.."\r\n\r\n");
                c:send(tmp);
        else
            tmp=getData.file.name.." not found";
            c:send("HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Length: "..#tmp.."\r\n\r\n");
            c:send(getData.file.name.." not found");
        end
        getData=nil;
        Header=nil;
    end  --end todoOnReceive
    
    local function todoOnSend(c)
        ThreadID=dofile("OnSend.lc")(ThreadID);
    end
    
    conn:on("receive",todoOnReceive)
    conn:on("sent",todoOnSend)
end)
