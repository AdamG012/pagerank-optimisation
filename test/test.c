#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <CUnit/Console.h>
#include <CUnit/Automated.h>
#include <assert.h>
#include "../src/pagerank.c"

int basic_cmp(void* a, void* b) {
	return *(int*)a - *(int*)b;
}

void basic_del(void* a) {
}


/******** NEW MAP METHOD TEST *****************/
void test_new_nullcmp(void) {
	CU_ASSERT_PTR_NULL(linkedlist_map_new(NULL, &basic_del, &basic_del));
}

void test_new_nullkeydel(void) {
	CU_ASSERT_PTR_NULL(linkedlist_map_new(&basic_cmp, NULL, &basic_del));
}


void test_new_nullvaldel(void) {
	CU_ASSERT_PTR_NULL(linkedlist_map_new(&basic_cmp, &basic_del, NULL));
}

void test_new_mapsize(void) {
	struct linkedlist_map* map = linkedlist_map_new(&basic_cmp, &basic_del, &basic_del);
	CU_ASSERT_EQUAL(linkedlist_map_size(map), 0);
	linkedlist_map_destroy(map);

}
/********************************************/


/******** PUT METHOD TEST ***********/

/**
 * Test where map parameter is null that it returns NULL
 */
void test_put_nullmap(void) {

	int data = 2;
	void* map = NULL;
	linkedlist_map_put(map, (void*)&data, (void*)&data);
	CU_ASSERT_PTR_NULL(map);
}

void test_put_nullkey(void) {
	int data = 2;
	void* map = linkedlist_map_new(&basic_cmp, &basic_del, &basic_del);
	linkedlist_map_put(map, NULL, (void*)&data);
	CU_ASSERT_EQUAL(linkedlist_map_size(map), 0);
}


void test_put_nullval(void) {
	int data = 2;
	void* map = linkedlist_map_new(&basic_cmp, &basic_del, &basic_del);
	linkedlist_map_put(map, (void*)&data, NULL);
	CU_ASSERT_EQUAL(linkedlist_map_size(map), 0);
}

void test_put_valid(void) {
	int* data = malloc(sizeof(int));
	int* key = malloc(sizeof(int));
	*data = 212;
	*key = 212;

	void* map = linkedlist_map_new(&basic_cmp, &basic_del, &basic_del);
	linkedlist_map_put(map, (void*)key, (void*)data);
	CU_ASSERT_EQUAL(linkedlist_map_size(map), 1);

	int* get_data = (int*)linkedlist_map_get(map, (void*)key);
	CU_ASSERT_EQUAL(*get_data, *key);
	linkedlist_map_destroy(map);
}

void test_put_valid_duplicate(void) {
	int* data = malloc(sizeof(int));
	int* key = malloc(sizeof(int));
	*data = 212;
	*key = 212;

	void* map = linkedlist_map_new(&basic_cmp, &basic_del, &basic_del);
	linkedlist_map_put(map, (void*)key, (void*)data);
	linkedlist_map_put(map, (void*)key, (void*)data);

	CU_ASSERT_EQUAL(linkedlist_map_size(map), 1);

	*key = 212;

	int* get_data = (int*)linkedlist_map_get(map, (void*)key);
	CU_ASSERT_EQUAL(*get_data, 212);
	linkedlist_map_destroy(map);
}


void test_put_valid_different(void) {
	int* data = malloc(sizeof(int));
	*data = 222;
	void* map = linkedlist_map_new(&basic_cmp, &basic_del, &basic_del);
	linkedlist_map_put(map, (void*)data, (void*)data);

	int* key = malloc(sizeof(int));
	*key = 3122;
	linkedlist_map_put(map, (void*)key, (void*)data);
	CU_ASSERT_EQUAL(linkedlist_map_size(map), 2);

	int* get_data = (int*)linkedlist_map_get(map, (void*)key);
	CU_ASSERT_EQUAL(*get_data, 222);
	free(get_data);
	linkedlist_map_destroy(map);
}
/* *********************************** */


/******** REMOVE METHOD TEST ***********/
void test_remove_nullmap(void) {
	int data = 2;
	CU_ASSERT_PTR_NULL(linkedlist_map_remove(NULL, (void*)&data));
}

void test_remove_nullkey(void) {
	void* map = linkedlist_map_new(&basic_cmp, &basic_del, &basic_del);
	CU_ASSERT_PTR_NULL(linkedlist_map_remove(map, NULL));
	linkedlist_map_destroy(map);
}


void test_remove_unknownkey(void) {
	int data = 2;
	void* map = linkedlist_map_new(&basic_cmp, &basic_del, &basic_del);
	CU_ASSERT_PTR_NULL(linkedlist_map_remove(map, (void*)&data));
	linkedlist_map_destroy(map);
}


void test_remove_valid_different(void) {

	int* data = malloc(sizeof(int));
	*data = 222;

	int* key = malloc(sizeof(int));
	*key = 3122;

	void* map = linkedlist_map_new(&basic_cmp, &basic_del, &basic_del);

	linkedlist_map_put(map, (void*)key, (void*)key);
	linkedlist_map_put(map, (void*)data, (void*)data);

	void* ret_val = linkedlist_map_remove(map, (void*)key);
	free(ret_val);
	CU_ASSERT_EQUAL(linkedlist_map_size(map), 1);

	linkedlist_map_destroy(map);

}
/* *********************************** */


/******** GET METHOD TEST ***********/
void test_get_nullmap(void) {
	int data = 2;
	CU_ASSERT_PTR_NULL(linkedlist_map_get(NULL, (void*)&data));
}

void test_get_nullkey(void) {
	void* map = linkedlist_map_new(&basic_cmp, &basic_del, &basic_del);
	CU_ASSERT_PTR_NULL(linkedlist_map_get(map, NULL));
	linkedlist_map_destroy(map);
}


void test_get_unknownkey(void) {
	int data = 2;
	void* map = linkedlist_map_new(&basic_cmp, &basic_del, &basic_del);
	CU_ASSERT_PTR_NULL(linkedlist_map_get(map, (void*)&data));
	linkedlist_map_destroy(map);
}
/* *********************************** */


/******** SIZE METHOD TEST ***********/
void test_size_nullmap(void) {
	CU_ASSERT_EQUAL(linkedlist_map_size(NULL), 0);
}

void test_size_emptymap(void) {
	void* map = linkedlist_map_new(&basic_cmp, &basic_del, &basic_del);
	CU_ASSERT_EQUAL(linkedlist_map_size(map), 0);
}

void test_size_nonzero(void) {
	int data = 2;
	void* map = linkedlist_map_new(&basic_cmp, &basic_del, &basic_del);
	linkedlist_map_put(map, (void*)&data, (void*)&data);
	CU_ASSERT_EQUAL(linkedlist_map_size(map), 1);
}
/* *********************************** */


/******** DESTROY METHOD TEST ***********/
void test_destroy_nullmap(void) {
	struct linkedlist_map* map = NULL;
	linkedlist_map_destroy(map);
	CU_ASSERT_PTR_NULL(map);
}


void test_destroy_emptymap(void) {
	struct linkedlist_map* map = linkedlist_map_new(&basic_cmp, &basic_del, &basic_del);
	linkedlist_map_destroy(map);
	CU_ASSERT_PTR_NULL(map);
}


void test_destroy_nonzero(void) {
	int data = 2;
	void* map = linkedlist_map_new(&basic_cmp, &basic_del, &basic_del);
	linkedlist_map_put(map, (void*)&data, (void*)&data);
	CU_ASSERT_PTR_NULL(map);
}


void test_destroy_valid(void) {

	int* data = malloc(sizeof(int));
	*data = 1337;
	int* key = malloc(sizeof(int));
	*key = 4423;
	void* map = linkedlist_map_new(&basic_cmp, &basic_del, &basic_del);

	linkedlist_map_put(map, (void*)key, (void*)key);
	linkedlist_map_put(map, (void*)data, (void*)data);

	void* ret_val = linkedlist_map_remove(map, (void*)key);
	free(ret_val);


	linkedlist_map_destroy(map);
	CU_ASSERT_EQUAL(linkedlist_map_size(map), 0);

}
/* *********************************** */

void* testcases[] = {
	&test_new_nullcmp,
	&test_new_nullkeydel,
	&test_new_nullvaldel,
	&test_put_nullmap,
	&test_put_nullkey,
	&test_put_nullval,
	&test_put_valid,
	&test_put_valid_duplicate,
	&test_put_valid_different,
	&test_remove_nullmap,
	&test_remove_nullkey,
	&test_remove_unknownkey,
	&test_remove_valid_different,
	&test_get_nullmap,
	&test_get_nullkey,
	&test_get_unknownkey,
	&test_size_nullmap,
	&test_size_emptymap,	
	&test_size_nonzero,
	&test_destroy_nullmap,
	&test_destroy_emptymap,
	&test_destroy_nonzero,
	&test_destroy_valid,
};

char* testcase_description[] = {
 	"test_new_nullcmp",
	"test_new_nullkeydel",
	"test_new_nullvaldel",
	"test_put_nullmap",
	"test_put_nullkey",
	"test_put_nullval",
	"test_put_valid",
	"test_put_get_valid_duplicate",
	"test_put_get_valid_different",
	"test_remove_nullmap",
	"test_remove_nullkey",
	"test_remove_unknownkey",
	"test_remove_get_valid_different",
	"test_get_nullmap",
	"test_get_nullkey",
	"test_get_unknownkey",
	"test_size_nullmap",
	"test_size_emptymap",	
	"test_size_nonzero",
	"test_destroy_nullmap",
	"test_destroy_emptymap",
	"test_destroy_nonzero",
	"test_destroy_valid",
};

int init_suite(void) {
	return 0;
}

int clean_suite(void) {
	return 0;
}

int main() {

	CU_pSuite p_suite = NULL;

	// Initalise the test registry
	if ( CUE_SUCCESS != CU_initialize_registry() )
		return CU_get_error();

	// Add a suite to the CUnit
	p_suite = CU_add_suite( "homework_test_suite", init_suite, clean_suite );
	if ( NULL == p_suite ) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	// Add all the tests to the suite
	for (int i = 0; i < 20; i++) {
		if (CU_add_test(p_suite, testcase_description[i], testcases[i]) == NULL) {
			CU_cleanup_registry();
			return CU_get_error();
		}
	}

	// Run all the tests setting verbose mode and showing all failures.
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_basic_show_failures(CU_get_failure_list());
	CU_cleanup_registry();
	return CU_get_error();

}
