#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <omp.h>

#include "pagerank.h"

#define END_ITER (5E-3 * 5E-3)


/**
 * The struct to store the page and its score
 */
struct page_score {
	double score[2];
	page* page;
	double difference;
	double filler[4];
};


/**
 * Clean up the allocated page_scores 
 * @param page_scores, the struct of page_scores
 */
void clean_up(void* page_scores) {
	if (page_scores == NULL) {
		return;
	}
	free(page_scores);
}


/**
 * Initialise the values of the struct array of page scores
 * @param plist, the list of pages
 * @param npages, the number of pages
 * @return the array of page score structs
 */
struct page_score* init_pageranks(list* plist, int npages) {
	struct page_score* page_scores = malloc(sizeof(struct page_score) * npages);
	register double initial_value = 1/(double)(npages);
	node* current = plist->head;
	for (size_t i = 0; i < npages; i++) {
		page_scores[i].page = current->page;
		page_scores[i].score[0] = initial_value;
		page_scores[i].score[1] = initial_value;
		page_scores[i].difference = 0;
		current = current->next;
	}
	return page_scores;
}

double update_score(struct page_score* page_scores, struct page_score* current_page, double dampener, int x) {

	double diff = 0.0;
	double total = 0.0;

	// Get the list of pages that inlink to this page
	list* inlist = (*current_page).page->inlinks;
	
	// If null then add diffrerence and continue looping
	if (inlist == NULL) {
		diff += ((*current_page).score[x] - (*current_page).score[!x]) * ((*current_page).score[x] - (*current_page).score[!x]);
		return diff;
	}

	// Get the node to loop
	node* current = inlist->head;

	// Loop through the list
	while (current != NULL) {
		total += (page_scores[current->page->index].score[!x]) / ((double)current->page->noutlinks);
		current = current->next;
	}

	(*current_page).score[x] += total * dampener; // Update the new score
	diff += ((*current_page).score[x] - (*current_page).score[!x]) * ((*current_page).score[x] - (*current_page).score[!x]);
	return diff;
}


/**
 * PageRank algorithm
 * Given a list of pages calculate the ranking of the pages using a dampening effect
 * @param plist, list of pages
 * @param ncores, number of cores
 * @param npages, number of pages
 * @param nedges, number of edges
 * @param dampener, the dampening effect on the pages
 */
void pagerank_unroll(list* plist, int ncores, int npages, int nedges, double dampener) {
	// Check for invalid parameters
	if (plist == NULL || ncores <= 0 || npages <= 0 || nedges < 0 || dampener <= 0) {
		return;
	}
	// Create page scores vector and load register for reused values
	struct page_score* page_scores = init_pageranks(plist, npages);
	double dampening_value = (1.0 - dampener)/((double)(npages));
	int x = 1;

	double diff = 1; // Used to check the difference of the scores

	// Loop through until the convergence threshold is reached
	while (diff > EPSILON) {
		diff = 0.0;
		size_t i = 0;
		for (; i <= npages - 4; i += 4) {
			page_scores[i].score[x] = dampening_value;
			diff += update_score(page_scores, &page_scores[i], dampener, x);

			page_scores[i + 1].score[x] = dampening_value;
			diff += update_score(page_scores, &page_scores[i + 1], dampener, x);

			page_scores[i + 2].score[x] = dampening_value;
			diff += update_score(page_scores, &page_scores[i + 2], dampener, x);

			page_scores[i + 3].score[x] = dampening_value;
			diff += update_score(page_scores, &page_scores[i + 3], dampener, x);
		}

		for (; i < npages; i++) {
			page_scores[i].score[x] = dampening_value;
			diff += update_score(page_scores, &page_scores[i], dampener, x);
		}

		x = (x + 1) % 2;	// Update the value so we do not have to copy
		diff = sqrt(diff);	// Get the total difference
	}
	
	// Print the results to stdout
	int i;
	for (i = 0; i <= npages - 4; i += 4) {
		printf("%s %.4lf\n", page_scores[i].page->name, page_scores[i].score[!x]);
		printf("%s %.4lf\n", page_scores[i + 1].page->name, page_scores[i + 1].score[!x]);
		printf("%s %.4lf\n", page_scores[i + 2].page->name, page_scores[i + 2].score[!x]);
		printf("%s %.4lf\n", page_scores[i + 3].page->name, page_scores[i + 3].score[!x]);
	}

	// Print the remaining results
	for (; i < npages; i++) {
		printf("%s %.4lf\n", page_scores[i].page->name, page_scores[i].score[!x]);
	}

	clean_up(page_scores);
}



/**
 * PageRank algorithm
 * Given a list of pages calculate the ranking of the pages using a dampening effect
 * @param plist, list of pages
 * @param ncores, number of cores
 * @param npages, number of pages
 * @param nedges, number of edges
 * @param dampener, the dampening effect on the pages
 */
void pagerank_nopow(list* plist, int ncores, int npages, int nedges, double dampener) {
	// Check for invalid parameters
	if (plist == NULL || ncores <= 0 || npages <= 0 || nedges < 0 || dampener <= 0) {
		return;
	}
	// Create page scores vector and load register for reused values
	struct page_score* page_scores = init_pageranks(plist, npages);
	register double dampening_value = (1.0 - dampener)/((double)(npages));
	register int x = 1;

	register double diff = 1; // Used to check the difference of the scores

	// Loop through until the convergence threshold is reached
	while (diff > EPSILON) {
		diff = 0.0;
		for (size_t i = 0; i < npages; i++) {
			page_scores[i].score[x] = dampening_value;
			register double total = 0.0;

			// Get the list of pages that inlink to this page
			list* inlist = page_scores[i].page->inlinks;
			
			// If null then add diffrerence and continue looping
			if (inlist == NULL) {
				diff += (page_scores[i].score[x] - page_scores[i].score[!x]) * (page_scores[i].score[x] - page_scores[i].score[!x]);
				continue;
			}

			// Get the node to loop
			node* current = inlist->head;

			// TODO can loop from back or loop unrolling
			// Loop through the list
			while (current != NULL) {
				total += (page_scores[current->page->index].score[!x]) / ((double)current->page->noutlinks);
				current = current->next;
			}
			page_scores[i].score[x] += total * dampener; // Update the new score
			diff += (page_scores[i].score[x] - page_scores[i].score[!x]) * (page_scores[i].score[x] - page_scores[i].score[!x]);
		}
		x = (x + 1) % 2;	// Update the value so we do not have to copy
		diff = sqrt(diff);	// Get the total difference
	}
	
	// Print the results to stdout
	for (int i = 0; i < npages; i++) {
		printf("%s %.4lf\n", page_scores[i].page->name, page_scores[i].score[!x]);
	}

	clean_up(page_scores);
}


/**
 * PageRank algorithm using the pow from math.h
 * Given a list of pages calculate the ranking of the pages using a dampening effect
 * @param plist, list of pages
 * @param ncores, number of cores
 * @param npages, number of pages
 * @param nedges, number of edges
 * @param dampener, the dampening effect on the pages
 */
void pagerank_pow(list* plist, int ncores, int npages, int nedges, double dampener) {
	// Check for invalid parameters
	if (plist == NULL || ncores <= 0 || npages <= 0 || nedges < 0 || dampener <= 0) {
		return;
	}
	// Create page scores vector and load register for reused values
	struct page_score* page_scores = init_pageranks(plist, npages);
	register double dampening_value = (1.0 - dampener)/((double)(npages));
	register int x = 1;

	register double diff = 1; // Used to check the difference of the scores

	// Loop through until the convergence threshold is reached
	while (diff > EPSILON) {
		diff = 0.0;
		for (size_t i = 0; i < npages; i++) {
			page_scores[i].score[x] = dampening_value;
			register double total = 0.0;

			// Get the list of pages that inlink to this page
			list* inlist = page_scores[i].page->inlinks;
			
			// If null then add diffrerence and continue looping
			if (inlist == NULL) {
				diff += pow(page_scores[i].score[x] - page_scores[i].score[!x], 2);;
				continue;
			}

			// Get the node to loop
			node* current = inlist->head;

			// TODO can loop from back or loop unrolling
			// Loop through the list
			while (current != NULL) {
				total += (page_scores[current->page->index].score[!x]) / ((double)current->page->noutlinks);
				current = current->next;
			}
			page_scores[i].score[x] += total * dampener; // Update the new score
			diff += pow(page_scores[i].score[x] - page_scores[i].score[!x], 2);
		}
		x = (x + 1) % 2;	// Update the value so we do not have to copy
		diff = sqrt(diff);	// Get the total difference
	}
	
	// Print the results to stdout
	for (int i = 0; i < npages; i++) {
		printf("%s %.4lf\n", page_scores[i].page->name, page_scores[i].score[!x]);
	}

	clean_up(page_scores);
}


/**
 * The struct to store the page and its score
 */
struct page_score_2D {
	double new_score;
	double old_score;
	page* page;
};


/**
* Initialise the values of the struct array of page scores using just a old and new double value
* @param plist, the list of pages
* @param npages, the number of pages
* @return the array of page score structs
*/
struct page_score_2D* init_pageranks_2D(list* plist, int npages) {
  struct page_score_2D* page_scores = malloc(sizeof(struct page_score_2D) * npages);
  register double initial_value = 1/(double)(npages);
  node* current = plist->head;
  for (size_t i = 0; i < npages; i++) {
	  page_scores[i].page = current->page;
	  page_scores[i].old_score = initial_value;
	  page_scores[i].new_score = 0;
	  current = current->next;
  }
  return page_scores;
}


/**
 * PageRank algorithm using the pow from math.h as well as the inefficient copy of the page_score struct
 * Given a list of pages calculate the ranking of the pages using a dampening effect
 * @param plist, list of pages
 * @param ncores, number of cores
 * @param npages, number of pages
 * @param nedges, number of edges
 * @param dampener, the dampening effect on the pages
 */
void pagerank_pow_old(list* plist, int ncores, int npages, int nedges, double dampener) {
	// Check for invalid parameters
	if (plist == NULL || ncores <= 0 || npages <= 0 || nedges < 0 || dampener <= 0) {
		return;
	}
	// Create page scores vector and load register for reused values
	struct page_score_2D* page_scores = init_pageranks_2D(plist, npages);
	register double dampening_value = (1.0 - dampener)/((double)(npages));
	register double diff = 1; // Used to check the difference of the scores

	// Loop through until the convergence threshold is reached
	while (diff > EPSILON) {
		diff = 0.0;
		for (size_t i = 0; i < npages; i++) {
			page_scores[i].new_score = dampening_value;
			register double total = 0.0;

			// Get the list of pages that inlink to this page
			list* inlist = page_scores[i].page->inlinks;
			
			// If null then add diffrerence and continue looping
			if (inlist == NULL) {
				diff += pow(page_scores[i].old_score - page_scores[i].old_score, 2);;
				continue;
			}

			// Get the node to loop
			node* current = inlist->head;

			// TODO can loop from back or loop unrolling
			// Loop through the list
			while (current != NULL) {
				total += (page_scores[current->page->index].old_score) / ((double)current->page->noutlinks);
				current = current->next;
			}
			page_scores[i].new_score += total * dampener; // Update the new score
			diff += pow(page_scores[i].new_score - page_scores[i].old_score, 2);
		}
		
		for (size_t i = 0; i < npages; i++) {
			page_scores[i].old_score = page_scores[i].new_score;
		}
		diff = sqrt(diff);	// Get the total difference
	}
	
	// Print the results to stdout
	for (int i = 0; i < npages; i++) {
		printf("%s %.4lf\n", page_scores[i].page->name, page_scores[i].new_score);
	}

	clean_up(page_scores);
}


struct page_score_padding {
	page* page;
	double score[16];
};


/**
 * Initialise the values of the struct array of page scores
 * @param plist, the list of pages
 * @param npages, the number of pages
 * @return the array of page score structs
 */
struct page_score_padding* init_pageranks_padding(list* plist, int npages) {
	struct page_score_padding* page_scores = malloc(sizeof(struct page_score_padding) * npages);
	register double initial_value = 1/(double)(npages);
	node* current = plist->head;
	for (size_t i = 0; i < npages; i++) {
		page_scores[i].page = current->page;
		page_scores[i].score[0] = initial_value;
		page_scores[i].score[15] = initial_value;
		current = current->next;
	}
	return page_scores;
}


/**
 * PageRank algorithm OpenMP
 * Given a list of pages calculate the ranking of the pages using a dampening effect
 * @param plist, list of pages
 * @param ncores, number of cores
 * @param npages, number of pages
 * @param nedges, number of edges
 * @param dampener, the dampening effect on the pages
 */
void pagerank_padding(list* plist, int ncores, int npages, int nedges, double dampener) {
	// Check for invalid parameters
	if (plist == NULL || ncores <= 0 || npages <= 0 || nedges < 0 || dampener <= 0) {
		return;
	}

	// Create page scores vector and load register for reused values
	struct page_score_padding* page_scores = init_pageranks_padding(plist, npages);
	double dampening_value = (1.0 - dampener)/((double)(npages));
	omp_set_num_threads(ncores);
	double diff = 1; // Used to check the difference of the scores
	int old_index = 15;
	int new_index = 0;

	// Loop through until the convergence threshold is reached

	while (diff > END_ITER) {
		diff = 0.0;
		size_t i;

		#pragma omp parallel for schedule(dynamic, 4) private(i)
		for (i = 0; i < npages; i++) {
			page_scores[i].score[new_index] = dampening_value;
			double total = 0.0;

			// Get the list of pages that inlink to this page
			list* inlist = page_scores[i].page->inlinks;
			
			// If null then add diffrerence and continue looping
			if (inlist == NULL) {
				continue;
			}

			// Get the node to loop
			node* current = inlist->head;

			// Loop through the list
			while (current != NULL) {
				total += (page_scores[current->page->index].score[old_index]) / ((double)current->page->noutlinks);
				current = current->next;
			}
			page_scores[i].score[new_index] += total * dampener; // Update the new score

			
		}

		#pragma omp parallel for reduction (+:diff)
		for (i = 0; i < npages; i++) {
			diff += (page_scores[i].score[new_index] - page_scores[i].score[old_index]) * (page_scores[i].score[new_index] - page_scores[i].score[old_index]);
		}

		old_index ^= new_index;
		new_index ^= old_index;
		old_index ^= new_index;
	}
	
	// Print the results to stdout
	#pragma omp parallel for ordered
	for (int i = 0; i < npages; i++) {
		// printf("%s %.4lf\n", page_scores[i].page->name, page_scores[i].score[!x]);
	}

	clean_up(page_scores);
}


// The ikj matrix multiply
void multiply(const double* mata, size_t mata_width, size_t mata_height,
        const double* matb, size_t matb_height,
        double* result_mat, double dampening_value) {

        if(result_mat) {
                size_t y, x;
                #pragma omp parallel shared(mata, matb, result_mat) private(y, x)
                {
                        #pragma omp for
                        for(y = 0; y < mata_height; y++) {
				result_mat[y] = dampening_value;
				for(x = 0; x < matb_height; x++) {
					(result_mat)[y] +=
					(matb[x] *
					 mata[(y * mata_width) + x]);
				}
                        }
                }
        }
}


/**
 * PageRank algorithm OpenMP
 * Given a list of pages calculate the ranking of the pages using a dampening effect
 * @param plist, list of pages
 * @param ncores, number of cores
 * @param npages, number of pages
 * @param nedges, number of edges
 * @param dampener, the dampening effect on the pages
 */
void pagerank_mm(list* plist, int ncores, int npages, int nedges, double dampener) {
	// Check for invalid parameters
	if (plist == NULL || ncores <= 0 || npages <= 0 || nedges < 0 || dampener <= 0) {
		return;
	}

	double* score_vector = malloc(sizeof(double) * npages);
	double* rank_vector = malloc(sizeof(double) * npages);
	double* matrix = calloc(sizeof(double), npages * npages);
	double dampening_value = (1.0 - dampener)/((double)(npages));
	double diff = 1; // Used to check the difference of the scores
	node* current = plist->head;
	
	for (int i = 0; i < npages; i++) {
		if (current->page->inlinks) {
			node* node = current->page->inlinks->head;
			while (node) {
				matrix[i * npages + node->page->index] = dampener/((double)node->page->noutlinks);
				node = node->next;
			}
		}
		current = current->next;
	}

	// Check if columns are 0 then set to 1/N
	#pragma omp parallel for shared(matrix, score_vector)
	for (int j = 0; j < npages; j++) {
		score_vector[j] = 1/((double)npages);

		double sum = 0;
		for (int i = 0; i < npages; i++) {
			if (matrix[(i * npages) + j]) {
				sum = 1;
				break;
			}
		}
		if (!sum) {
			for (int i = 0; i < npages; i++) {
				matrix[i * npages + j] = dampener/((double)npages);
			}
		}
	}

	/*
	 * PRINTING VALUES OF MATRIX
	for (int i = 0; i < npages; i++) {
		for (int j = 0; j < npages; j++) {
			printf("%lf ", matrix[i * npages + j]);
		}
		printf("\n");
	}
	for (int i = 0; i < npages; i++) {
		printf("%lf\n", score_vector[i]);
	}
	*/

	omp_set_num_threads(ncores);

	// Loop through until the convergence threshold is reached
	while (diff > EPSILON) {
		diff = 0.0;
		multiply(matrix, npages, npages, score_vector, npages, rank_vector, dampening_value);

		#pragma omp parallel for reduction (+:diff)
		for (int i = 0; i < npages; i++) {
			diff += (rank_vector[i] - score_vector[i]) * (rank_vector[i] - score_vector[i]);
		}
		diff = sqrt(diff);

		/*
		#pragma omp parallel for
		for (int i = 0; i < npages; i++) {
			rank_vector[i] = rank_vector[i] / norm;
		}
		*/

		// Update for next iteration so that we do not have to copy
		double* temp = rank_vector;
		rank_vector = score_vector;
		score_vector = temp;
	}
	
	// Print the results to stdout
	current = plist->head;
	for (int i = 0; i < npages; i++) {
		 printf("%s %.4lf\n", current->page->name, score_vector[i]);
		 current = current->next;
	}

	free(score_vector);
	free(rank_vector);
	free(matrix);
}



/**
 * PageRank algorithm OpenMP
 * Given a list of pages calculate the ranking of the pages using a dampening effect
 * @param plist, list of pages
 * @param ncores, number of cores
 * @param npages, number of pages
 * @param nedges, number of edges
 * @param dampener, the dampening effect on the pages
 */
void pagerank(list* plist, int ncores, int npages, int nedges, double dampener) {
	// Check for invalid parameters
	if (plist == NULL || ncores <= 0 || npages <= 0 || nedges < 0 || dampener <= 0) {
		return;
	}

	// Create page scores vector and load register for reused values
	struct page_score* page_scores = init_pageranks(plist, npages);
	double dampening_value = (1.0 - dampener)/((double)(npages));
	register int x = 1;
	omp_set_num_threads(ncores);

	register double diff = 1; // Used to check the difference of the scores

	// Loop through until the convergence threshold is reached
	while (diff > 0.000025) {
		diff = 0.0;
		size_t i;

		#pragma omp parallel for private(i)
		for (i = 0; i < npages; i++) {
			page_scores[i].score[x] = dampening_value;
			double total = 0.0;

			// Get the list of pages that inlink to this page
			list* inlist = page_scores[i].page->inlinks;
			
			// If null then add diffrerence and continue looping
			if (inlist == NULL) {
				page_scores[i].difference = (page_scores[i].score[x] - page_scores[i].score[!x]) * (page_scores[i].score[x] - page_scores[i].score[!x]);
				continue;
			}
			// Get the node to loop
			node* current = inlist->head;

			// Loop through the list
			while (current != NULL) {
				total += (page_scores[current->page->index].score[!x]) / ((double)current->page->noutlinks);
				current = current->next;
			}
			page_scores[i].score[x] += total * dampener; // Update the new score
			page_scores[i].difference = (page_scores[i].score[x] - page_scores[i].score[!x]) * (page_scores[i].score[x] - page_scores[i].score[!x]);
		}

		#pragma omp parallel for reduction (+:diff)
		for (i = 0; i < npages; i++) {
			diff += page_scores[i].difference;
		}

		x = (x + 1) % 2;	// Update the value so we do not have to copy
	}
	
	// Print the results to stdout
	for (int i = 0; i < npages; i++) {
		// printf("%s %.4lf\n", page_scores[i].page->name, page_scores[i].score[!x]);
	}

	clean_up(page_scores);
}


/*
######################################
### DO NOT MODIFY BELOW THIS POINT ###
######################################
*/

int main(void) {

    /*
    ######################################################
    ### DO NOT MODIFY THE MAIN FUNCTION OR HEADER FILE ###
    ######################################################
    */

    list* plist = NULL;

    double dampener;
    int ncores, npages, nedges;

    /* read the input then populate settings and the list of pages */
    read_input(&plist, &ncores, &npages, &nedges, &dampener);

    double start = omp_get_wtime();
    pagerank(plist, ncores, npages, nedges, dampener);
    double end = omp_get_wtime();
    printf("%lf\n", end - start);

    /* clean up the memory used by the list of pages */
    page_list_destroy(plist);

    return 0;
}
