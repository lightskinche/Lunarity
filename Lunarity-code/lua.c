#include "main_h.h"

audio_component* COMPONENT_CreateAudioComponent(const char* name, Mix_Chunk* data) {
	audio_component* tmp_audio = calloc(1, sizeof(audio_component));
	tmp_audio->name = name, tmp_audio->audio_data = data;
	int i;
	for (i = 0; name[i] != '_'; ++i);
	tmp_audio->name_size = i + 10, tmp_audio->playing = 0;
	return tmp_audio;
}

void TRANSFORM_AddComponent(int index, void* component, Uint8 type, transform* transform_to_attach) {
	transform_to_attach->components[index] = component;
	transform_to_attach->component_order[index] = type;
}
void TRANSFORM_SetSize(transform* transform_to_modify, int size) {
	transform_to_modify->components = calloc(size, sizeof(void*));
	transform_to_modify->component_order = calloc(size, sizeof(Uint8));
	transform_to_modify->size_components = size;
}