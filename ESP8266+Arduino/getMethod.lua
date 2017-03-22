local function checkPath(thisPath)
    local fileinfo={}
    if thisPath=="/" then
        fileinfo.name="index.htm"
        fileinfo.contenttype="text/html"
    elseif thisPath:find("&no",1,true) then
        fileinfo.name="runCommand";
        fileinfo.value=thisPath:sub(thisPath:find("?",1,true)+1,#thisPath);
    elseif thisPath:find("update",1,true) then
        fileinfo.name="update";
        if thisPath:find("createFile",1,true) then
            fileinfo.start=thisPath:sub(thisPath:find("totalRec=",1)+9,#thisPath);
        else 
            fileinfo.curRec=thisPath:sub(thisPath:find("curRec=",1)+7,thisPath:find("&",8)-1);
            fileinfo.totalRec=thisPath:sub(thisPath:find("totalRec=",1)+9,thisPath:find("&",20)-1);
            fileinfo.value=thisPath:sub(thisPath:find("JSON=",11)+5,#thisPath);
            fileinfo.value=string.gsub(fileinfo.value,"(%%22)","\"");
            fileinfo.value=string.gsub(fileinfo.value,"(%%20)"," ");
            if fileinfo.curRec==fileinfo.totalRec then
                fileinfo.value=fileinfo.value.."\r\n]";
            else
                fileinfo.value=fileinfo.value..",";
            end
        end        
    elseif thisPath:find("luaCMD?",1,true) then
        fileinfo.name="luaCMD";
        fileinfo.value=thisPath:sub(thisPath:find("luaCMD?",1)+7,#thisPath);
    elseif thisPath:find("&fromArduino",1,true) then
        fileinfo.name=thisPath:sub(2,thisPath:find("&",1,true)-1);
        print("["..fileinfo.name.."]");
        fileinfo.value="fromArduino";
    else
        fileinfo.name=thisPath:sub(2,#thisPath);
    end
    fileinfo.size=#fileinfo.name;    
    if fileinfo.name~="ajax" then
        if file.open(fileinfo.name,r) then
            fileinfo.size=file.seek("end");
            file.close();
            fileExist="true";
        else
            fileExist="false";
        end
    end
    return fileinfo,fileExist
end

return function (payload)
    local firstEOL=payload:find("\r\n",1,true);
    if not firstEOL then return nil end;
    local firstLine=payload:sub(1,firstEOL-1);
    local request={};
    _, i, request.method, request.path = firstLine:find("^([A-Z]+) (.-) HTTP/[1-9]+.[0-9]+$")    
    request.file,request.fileExist=checkPath(request.path);
    return request;
end
