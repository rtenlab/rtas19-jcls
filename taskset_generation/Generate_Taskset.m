function [ sets, util_M, util_m, valid ] = Generate_Taskset( num, t_range, k_range, util, mode )
    % mode (1) : apply same (m, k) constraints for all tasks
    
    % T / C / D / m / k / offset / sporadic / jitter
    sets = zeros(num, 5);
    
    T = randi(t_range, num, 1);
    sets(:, 1) = T';
    sets(:, 3) = T';
    
    util_per = UUniFast(num, util);
    valid = true;
    if ~isempty(find(util_per > 1.0))
%         disp('Not correct generation');
        valid = false;
    end
    util_M = 0; util_m = 0;
    
    K = randi(k_range, 1, 1);
    M = randi([1 9], 1, 1);
    for i = 1 : num
        sets(i, 2) = round(util_per(i)*sets(i, 1),1); 
        if mode ~= 1
            sets(i, 5) = randi(k_range, 1, 1);
            sets(i, 4) = randi([1 k_range(1)-1], 1, 1);
        else
            sets(i, 5) = K;
            sets(i, 4) = M;
        end
        sets(i, 6) = 0;
        sets(i, 7) = 0;
        sets(i, 8) = 0;
        
        util_M = util_M + sets(i, 2)/sets(i, 1);
        util_m = util_m + sets(i, 2)/sets(i, 1)*sets(i, 4)/sets(i, 5);
    end
    
end

