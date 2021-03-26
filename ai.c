#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <math.h>

#include "ai.h"
#include "utils.h"
#include "hashtable.h"
#include "stack.h"


void copy_state(state_t* dst, state_t* src){
	
	//Copy field
	memcpy( dst->field, src->field, SIZE*SIZE*sizeof(int8_t) );

	dst->cursor = src->cursor;
	dst->selected = src->selected;
}

/**
 * Saves the path up to the node as the best solution found so far
*/
void save_solution( node_t* solution_node ){
	node_t* n = solution_node;
	while( n->parent != NULL ){
		copy_state( &(solution[n->depth]), &(n->state) );
		solution_moves[n->depth-1] = n->move;

		n = n->parent;
	}
	solution_size = solution_node->depth;
}


node_t* create_init_node( state_t* init_state ){
	node_t * new_n = (node_t *) malloc(sizeof(node_t));
    assert(new_n);

	new_n->parent = NULL;	
	new_n->depth = 0;
	copy_state(&(new_n->state), init_state);
	return new_n;
}

/**
 * Apply an action to node n and return a new node resulting from executing the action
 * This function is written by Daniel Zheng. student ID: 1111121
*/
node_t* applyAction(node_t* n, position_s* selected_peg, move_t action ){

    node_t* new_node = NULL;

    new_node = create_init_node(&n->state);
    new_node->depth = n->depth + 1;
    new_node->parent = n;
    new_node->move = action;
    new_node->state.cursor.x = selected_peg->x;
    new_node->state.cursor.y = selected_peg->y;

    execute_move_t( &(new_node->state), &(new_node->state.cursor), action );
	
	return new_node;

}

/**
 * Free all the node that exploered by the algorithm
 */
void free_node(node_t *node) {
    if (node == NULL) {
        return;
    }
    if (node->parent != NULL) {
        free_node(node->parent);
    }
    free(node);
}

/**
* Free all the poped node
*/
void free_poped(node_t * node, node_t *top_stack_node) {
    if (node->parent != top_stack_node->parent) {
        free_poped(node->parent, top_stack_node);
    }
    free(node);
}

/**
* Free all the left used memory
*/
void free_memory(node_t *node, position_s *coor, HashTable *table) {
    free_node(node);
    free(coor);
    free_stack();
    ht_destroy(table);
}


/**
 * Find a solution path as per algorithm description in the handout
 * This function is written by Daniel Zheng. student ID: 1111121
 */
void find_solution( state_t* init_state  ){

	HashTable table;

	// Choose initial capacity of PRIME NUMBER 
	// Specify the size of the keys and values you want to store once 
	ht_setup( &table, sizeof(int8_t) * SIZE * SIZE, sizeof(int8_t) * SIZE * SIZE, 16769023);

	// Initialize Stack
	initialize_stack();

	//Add the initial node
	node_t* n = create_init_node( init_state );
	
	//FILL IN THE GRAPH ALGORITHM
    stack_push(n);

    // Initialize and declar all the variable
    node_t *new_node;
    move_t jump;
    int value = 0, child_notfound;    // This value of 'value' is numerically meaningless
    int remainingPegs = num_pegs(&(n->state));
    int8_t x_coor, y_coor;
    position_s *selected_peg = (position_s *)malloc(sizeof(position_s));
    assert(selected_peg);


    // main implmention of algorithem
    while( !is_stack_empty() ){

        n = stack_top(); stack_pop();
        expanded_nodes += 1;

        if ( num_pegs(&(n->state)) < remainingPegs ){
            save_solution( n );
            remainingPegs = num_pegs(&(n->state));
        }

        child_notfound = 1;
        for ( x_coor = 0; x_coor < SIZE; x_coor++ ) {
            for ( y_coor = 0; y_coor < SIZE; y_coor++) {
                selected_peg->x = x_coor; selected_peg->y = y_coor;
                for( jump = left; jump <= down; jump++ ) {

                    /* find a better solution */
                    if ( can_apply( &(n->state), selected_peg, jump ) ){
                        /* create a child node */
                        new_node = applyAction( n, selected_peg, jump );
                        generated_nodes += 1;

                        /* Peg Solitaire Soleved */
                        if ( won( &(new_node->state) ) ){
                            save_solution( new_node );
                            remainingPegs = num_pegs(&(new_node->state));
                            free_memory(new_node, selected_peg, &table);
                            return;
                        }
                        /* avoid duplicates */
                        if ( !ht_contains(&table, &new_node->state) ){
                            /* Depth First Search */
                            stack_push( new_node );
                            ht_insert(&table, &new_node->state, &value);
                            child_notfound = 0;
                        }
                        else {
                            free(new_node);
                        }
                    }
                }
            }
        }

        /* free the node that has been poped out */
        if (child_notfound) {
            if ( is_stack_empty() ) {
                free_node(n);
            } else {
                free_poped(n, stack_top());
            }
        }

        /* Budget exhausted */
        if ( expanded_nodes >= budget ){
            /* prevent the case that the stack is empty */
            if ( is_stack_empty() ) {
                free_memory(NULL, selected_peg, &table);
            } else {
                new_node = stack_top(); stack_pop();
                free_memory(new_node, selected_peg, &table);
            }
            return;
        }
    }

    /* Program ended free all the structures allocated */
    free_memory(NULL, selected_peg, &table);
}
