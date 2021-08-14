#include "jcls.h"

//int num_tasks;
int num_classes;

unsigned int find_index (unsigned int *arr_in, unsigned int arr_in_idx, unsigned int *arr_out, unsigned int val) {
	int i, arr_out_idx = 0;
	int size;
	for (i = 0; i < arr_in_idx; i++) {
		if (arr_in[i] == val) {
			arr_out[arr_out_idx] = i;
			arr_out_idx++;
		}
	}
	return arr_out_idx;
}

unsigned int count_seq(char *arr_in, char val) {
	size_t size;
	int i;
	unsigned int cnt = 0;
	size = strlen(arr_in);
	for (i = 0; i < size; i++) {
		if (arr_in[i] == val) {
			cnt++;
		}
	}
	return cnt;
}

int schedulability (struct task *tasks, unsigned int num_tasks) {
	int i, j, n, l, q, t, y, M, K, cls_idx, flag, sched_flag;
	unsigned int p_cls, p_miss, m_count;
	unsigned int *CLS, *index, *list, size, size_list;
	char *p_seq, *tmp_seq, *tmp_p_seq;
	struct tree_node node;
	
	sched_flag = 1;

	// Allocate memory
	CLS = malloc(sizeof(unsigned int)*MAX_CLS_SIZE);
	index = malloc(sizeof(unsigned int)*MAX_CLS_SIZE);
	list = malloc(sizeof(unsigned int)*MAX_CLS_SIZE);  

	p_seq = (char *)malloc(sizeof(char)*MAX_SEQUENCE);
	tmp_seq = (char *)malloc(sizeof(char)*MAX_SEQUENCE);
	tmp_p_seq = (char *)malloc(sizeof(char)*MAX_SEQUENCE);

	node.W = malloc(sizeof(unsigned int)*MAX_CLS_SIZE);
	node.cls = malloc(sizeof(unsigned int)*MAX_CLS_SIZE);
	node.miss = malloc(sizeof(unsigned int)*MAX_CLS_SIZE);
	node.seq = (char **)malloc(sizeof(char *)*MAX_CLS_SIZE);
	for(i = 0; i < MAX_CLS_SIZE; i++) {
		node.seq[i] = (char *)malloc(sizeof(char)*MAX_SEQUENCE);
	}

	// Schedulability analysis start
	for (i = 0; i < num_tasks; i++) {
		M = tasks[i].m;
		K = tasks[i].K;
		cls_idx = 0;
		for (j = 0; j < tasks[i].m+1; j++) {
			if (tasks[i].WCRT[j] <= tasks[i].T-tasks[i].jitter) {
				CLS[cls_idx] = 1;
				cls_idx++;
			} else {
				CLS[cls_idx] = 0;
				cls_idx++;
			}
		}

		// Generate reachability trees
		for (n = 0; n < tasks[i].m+1; n++) {
			
			// Check the ratio of m/K and condition
			if ((M/((double)K)) <= 0.5 && CLS[0] == 1) {
				#ifdef DEBUG_SCHED
					printf("M is %d, K is %d\n", M, K);
					printf("M/K ratio is %lf\n", (M/((double)K)));
					printf("m/K ratio is less than 0.5.\n");
				#endif		
				break;
			}
			
			// Generate node
			init_node(&node, n, 0);
			
			tmp_seq = malloc(sizeof(char)*MAX_SEQUENCE);
			for (l = 0; l < n+1; l++) {
				if (l == 0) {
					strcat(tmp_seq, "0");
				} else {
					strcat(tmp_seq, "1");
				}
			}
			node.seq[node.seq_idx] = tmp_seq;
			node.seq_idx++;

			
			for (q = 1; q < K; q++) {
				size = find_index(node.W, node.W_idx, index, q-1);

				for (t = 0; t < size; t++) {
					p_cls = node.cls[index[t]];
					p_seq = node.seq[index[t]];
					p_miss = node.miss[index[t]];
					flag = 2;
					
					// Generate cases
					if (p_cls >= cls_idx)
						p_cls = cls_idx - 1;

					if (CLS[p_cls] != 1) {
						
						while (flag) {
							node.W[node.W_idx] = q;
							node.W_idx++;
							
							if (flag == 2) {
								node.cls[node.cls_idx] = 0;
								node.cls_idx++;

								tmp_p_seq = malloc(sizeof(char)*MAX_SEQUENCE);
								strcpy(tmp_p_seq, p_seq);
								strcat(tmp_p_seq, "0");
								node.seq[node.seq_idx] = tmp_p_seq;
								node.seq_idx++;
								
								node.miss[node.miss_idx] = p_miss+1;
								node.miss_idx++;								
							} else {
								if (p_cls == (cls_idx - 1)) {
									node.cls[node.cls_idx] = p_cls;
									node.cls_idx++;
								} else {
									node.cls[node.cls_idx] = p_cls+1;
									node.cls_idx++;
								}
								node.miss[node.miss_idx] = p_miss;
								node.miss_idx++;
								
								tmp_p_seq = malloc(sizeof(char)*MAX_SEQUENCE);
								strcpy(tmp_p_seq, p_seq);
								strcat(tmp_p_seq, "1");
								node.seq[node.seq_idx] = tmp_p_seq;
								node.seq_idx++;															
							}
							
							flag = flag - 1;
						}
					} else {
						node.W[node.W_idx] = q;
						node.W_idx++;

						if (p_cls >= cls_idx - 1) {
							node.cls[node.cls_idx] = p_cls;
							node.cls_idx++;
						} else {
							node.cls[node.cls_idx] = p_cls+1;
							node.cls_idx++;
						}
						node.miss[node.miss_idx] = p_miss;
						node.miss_idx++;

						tmp_p_seq = malloc(sizeof(char)*MAX_SEQUENCE);
						strcpy(tmp_p_seq, p_seq);
						strcat(tmp_p_seq, "1");
						node.seq[node.seq_idx] = tmp_p_seq;
						node.seq_idx++;					
					}
				}
			}
			
			#ifdef DEBUG_SCHED
				if (i == 0 && n == 0) {
					for (int b = 0; b < node.seq_idx; b++)
						printf("Node seq is %s\n", node.seq[b]);	
				}
			#endif
			
			size_list = find_index(node.W, node.W_idx, list, tasks[i].K-1);
			#ifdef DEBUG_SCHED
				printf("size_list is %d\n", size_list);
			#endif
			for (y = 0; y < size_list; y++) {
				m_count = count_seq(node.seq[list[y]], '1');
				if (m_count < tasks[i].m) {
					//printf("Error check point 1\n");
					// unschedulable
					#ifdef DEBUG_SCHED			
						for(i = 0; i < MAX_CLS_SIZE; i++) {
							free(node.seq[i]);
						}
					#endif
					
					sched_flag = 0;
					break;
				}
			}

			if (sched_flag == 0)
				break;
		}
	}


	// Allocate memory
	CLS = malloc(sizeof(unsigned int)*MAX_CLS_SIZE);
	index = malloc(sizeof(unsigned int)*MAX_CLS_SIZE);
	list = malloc(sizeof(unsigned int)*MAX_CLS_SIZE);  

	p_seq = (char *)malloc(sizeof(char)*MAX_SEQUENCE);
	tmp_seq = (char *)malloc(sizeof(char)*MAX_SEQUENCE);
	//tmp_p_seq = (char *)malloc(sizeof(char)*MAX_SEQUENCE);

	node.W = malloc(sizeof(unsigned int)*MAX_CLS_SIZE);
	node.cls = malloc(sizeof(unsigned int)*MAX_CLS_SIZE);
	node.miss = malloc(sizeof(unsigned int)*MAX_CLS_SIZE);
	node.seq = (char **)malloc(sizeof(char *)*MAX_CLS_SIZE);



	free(CLS);
	free(index);
	free(list);

	free(node.seq);
	free(node.miss);
	free(node.cls);
	free(node.W);

	free(p_seq);
	free(tmp_seq);
	free(tmp_p_seq);

	if (sched_flag == 1)
		return 1;
	else
		return 0;
}