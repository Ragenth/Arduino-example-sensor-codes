return function (ThreadID)
    if ThreadID then
        local ThreadStatus=coroutine.status(ThreadID)
        if ThreadStatus=="suspended" then
            local status, err=coroutine.resume(ThreadID);
            if not status then
                print("error="..err);
            end
            return ThreadID;
        elseif ThreadStatus=="dead" then
                collectgarbage();
            return nil;
        end
    end
end
