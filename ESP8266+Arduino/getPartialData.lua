return function (connection,myFile)
--    print("file to send="..myFile.name);
--    print("filesize="..myFile.size);
    local data_available=true;
    local currentPos=0;
    local partialData={};
    partialData.size=0;
    partialData.data={};
    function partialData:send(payload) 
        local l = payload:len()
        if l + self.size > 1000 then
            if self:flush() then
                coroutine.yield()
            end
        end
        if l > 700 then
            connection:send(payload)
            coroutine.yield()
        else
            table.insert(self.data, payload)
            self.size = self.size + l
        end
    end
    function partialData:flush() 
    if self.size > 0 then 
        connection:send(table.concat(self.data, ""))
        self.data = {}
        self.size = 0    
        return true
    end
        return false
    end     
    while data_available do
        file.open(myFile.name,r);
        file.seek("set",currentPos);
        local partial_data=file.read(400);
        file.close();
        if partial_data==nil then
--            print("partial_data_nil");
            data_available=false;
            if partialData.data~=nil then
                connection:send(table.concat(partialData.data, ""))
            end
            partialData.data = {}
            partialData.size = 0
        else
            partialData:send(partial_data)
            currentPos=currentPos+#partial_data;
            print("file "..myFile.name.." "..currentPos.."/"..myFile.size.." bytes");
            partial_data=nil;
--            tmr.wdclr();
        end
--        collectgarbage();        
    end
end
