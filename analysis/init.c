#include "jcls.h"

int num_classes;

int load_data(FILE *fp, long double **data, unsigned int k) {
	int i, j;

	// Load txt file data into array
	for(i = 0; i < k; i++) {
		for(j = 0; j < 8; j++) {
			fscanf(fp, "%Lf", &data[i][j]);
		}
	}

	return 0;
}

// Initialize task (C, T, m, K, jitter)
void initialize_task(struct task *t, long double C, long double T, unsigned int m, unsigned int K, long double jitter) {
	t->C = C;
	t->T = T;
	t->m = m;
	t->K = K;
	t->jitter = jitter;

	t->w = MAX(floor((double)K/(K-(K-m))) - 1, 1);
	t->v = ceil(( (double)(K-(K-m)) / (double)(K-m) ));
	num_classes = num_classes + (m + 1);
	t->min_utilization = C/T*m/K;
	t->utilization = C/T;
	t->cpu = (unsigned int *)malloc((m+1)*sizeof(unsigned int));
	t->eta = (double *)malloc((m+1)*sizeof(double));
	for (int i = 0; i < m+1; i++) {
		t->cpu[i] = 0; 
		t->eta[i] = 0.0;
	}
		
}

void init_node(struct tree_node *node, unsigned int init_cls, unsigned int init_miss) {
	node->W_idx = 0;
	node->cls_idx = 0;
	node->miss_idx = 0;
	node->seq_idx = 0;

	node->W[node->W_idx] = 0;
	node->W_idx++;
	node->cls[node->cls_idx] = init_cls;
	node->cls_idx++;
	node->miss[node->miss_idx] = init_miss;
	node->miss_idx++;
}