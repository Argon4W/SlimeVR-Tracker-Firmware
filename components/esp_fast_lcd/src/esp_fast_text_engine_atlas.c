#include "string.h"
#include "esp_fast_text_engine.h"
#include "esp_fast_text_engine_common.h"

void private_move_lru_node_to_head(
	esp_fast_text_engine_lru_atlas_t*	atlas,
	esp_fast_text_engine_lru_node_t*	node
) {
	// Skip if the node is already the head node.
	if (node == atlas->head) {
		return;
	}

	if (node == atlas->tail) {
		// Update the tail node pointer.
		atlas->tail = node->prev;

		// Detach the current node from previous node.
		node->prev->next	= NULL;
		node->prev			= NULL;
	} else {
		// Attach the previous node to the next node.
		node->prev->next = node->next;
		node->next->prev = node->prev;

		// Detach current node from the previous node and next node.
		node->prev = NULL;
		node->next = NULL;
	}

	// The old head node of the linked list.
	esp_fast_text_engine_lru_node_t *head = atlas->head;

	// Attach current node to the head of the linked list.
	node->next = head;
	head->prev = node;

	// Update the head node pointer.
	atlas->head = node;
}

void private_move_lru_node_to_tail(
	esp_fast_text_engine_lru_atlas_t*	atlas,
	esp_fast_text_engine_lru_node_t*	node
) {
	// Skip if the node is already the tail node.
	if (node == atlas->tail) {
		return;
	}

	if (node == atlas->head) {
		// Update the head node pointer.
		atlas->head = node->next;

		// Detach the current node from next node.
		node->next->prev	= NULL;
		node->next			= NULL;
	} else {
		// Attach the previous node to the next node.
		node->prev->next = node->next;
		node->next->prev = node->prev;

		// Detach current node from the previous node and next node.
		node->prev = NULL;
		node->next = NULL;
	}

	// The old tail node of the linked list.
	esp_fast_text_engine_lru_node_t *tail = atlas->tail;

	// Append current node to the tail of the linked list.
	node->prev = tail;
	tail->next = node;

	// Update the tail node pointer.
	atlas->tail = node;
}