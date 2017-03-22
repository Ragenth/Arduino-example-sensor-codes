local CoRoutine={};

function CoRoutine.create(c,myFile)
    CoRoutine.ThreadID=coroutine.create(function(c,myFile)
        collectgarbage();
        print(node.heap());
        print("on coroutine");
        local getPartialData=require("getPartialData")
        getPartialData.dowhile();
--        dofile("getPartialData.lc")(c,myFile)
    end) 
    local status, err=coroutine.resume(CoRoutine.ThreadID,c,myFile);
    if not status then
        print("error="..err);
    end
end

return CoRoutine
