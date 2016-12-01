// #define TIMINGS

void fillRowAddMatrix(int A[MERGE_LEVEL_MAX][MERGE_LEVEL_MAX], filter_matrix_t *mat, int m, int32_t *ind, int32_t ideal);
int minimalSpanningTree(int *start, int *end, int m, int A[MERGE_LEVEL_MAX][MERGE_LEVEL_MAX]);
int minCostUsingMST(filter_matrix_t *mat, int m, int32_t *ind, int32_t j);
void printMST(int *father, int *sons, int m, int32_t *ind);
