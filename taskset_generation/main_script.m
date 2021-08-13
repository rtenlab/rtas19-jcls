%% Experiments
% Generate task sets
clc; close all; clear all;

sets = {};
N = 2000; % Number of tasksets of specific utilization
N_range = [20 20]; % The range of tasks
u_start = 8; u_finish = 8; unit = 0.05;
M = ceil((u_finish - u_start)/unit)+1;
if M == 0 M = 1; end
U = linspace(u_start, u_finish, M);
iter = 0;
for u = 1 : M
    tmp_util_M = 0; tmp_util_m = 0; valid_task = 0; flag = 1; i = 1;
    cnt = 0;
    k = 0;
    while flag
        rm_flag = 1;
        iter = iter + 1;
        valid_task = 1;
        
        % Generate task set
        % num of tasks, T range, K range, Maximum utilization
        T = randi(N_range, 1);
        [Data_raw, util_M, util_m, valid] = Generate_Taskset(T, [10 1000], [10 10], U(u), 0);
        Data = sortrows(Data_raw, [1, 2], {'ascend' 'descend'});
        ss = size(Data);
        
        % Define tasks
        for j = 1 : ss(1, 1)
            task(j) = MKTaskModel(j, Data(j,1), Data(j,2), Data(j,3), Data(j,4), Data(j,5), Data(j,6), Data(j,7), Data(j,8));
        end     
        wcrt = WCRT_rm(task);
%         
        for j = 1 : ss(1, 1)
            if strcmp(wcrt(j).schedulable, 'un-schedulable')
                rm_flag = 0;
%                 valid_task = 0;
            end
        end
        
        if rm_flag == 1
            cnt = cnt + 1;
        end
        
        if valid == true
            sets(u, i).util_M = util_M;
            sets(u, i).util_m = util_m;
            tmp_util_M = tmp_util_M + util_M;
            tmp_util_m = tmp_util_m + util_m;
            sets(u, i).Data = Data;
            i = i + 1
        end
        
        if i == N + 1
            flag = 0;
        end
        k = k + 1;
    end
    rm_sched(u) = cnt/N;
    Max_util(u) = tmp_util_M/N;
    Min_util(u) = tmp_util_m/N;
    
    disp(['Max Utilization of ',num2str(U(u)),': ',num2str(tmp_util_M/N)]);
    disp(['Min Utilization of ',num2str(U(u)),': ',num2str(tmp_util_m/N)]);
end
% save('multicore_util_8_N20.mat', 'sets', 'U', 'Max_util','Min_util', 'rm_sched', 'M','N');
