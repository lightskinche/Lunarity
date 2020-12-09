#include "main_h.h"
void* LIST_At(const linkedList* const list, int location);
//DO NOT UPDATE THESE, THERE ARE TOO MANY CHANGES GOING ON IN THE INTERNAL ENGINE, IT IS A WASTE TO KEEP CHANGING THIS OVER AND OVER
void SERALIZE_WriteTransform(const char* fname, transform* input) {
	FILE* fp = fopen(fname, "w");
	fprintf(fp, "%x\n%x\n", input->x, input->y);
	if (input->attached_script) {
		fprintf(fp, "%x\n%x\n", 1, input->attached_script->resource_id);
	} 
	//print out the flags before we get into the components
	fprintf(fp, "%x\n", input->flags);
	if (input->renderable_shape) {
		if (input->flags & RENDERABLE_SQUARE) {
			renderable_square* tmp_renderablesq = input->renderable_shape;
			fprintf(fp, "%x\n%x\n", tmp_renderablesq->w, tmp_renderablesq->h);

		}

	}
	fprintf(fp, "%x\n", input->size_components);
	for (int i = 0; i < input->size_components; ++i) {
		fprintf(fp, "%x\n", input->component_order[i]);
		if (input->component_order[i] == COMPONENT_AUDIO_PLAYER) {
			audio_component* tmp_audio = input->components[i];
			fprintf(fp, "%x\n", tmp_audio->name_size);
			for (int i = 0; i < tmp_audio->name_size; ++i) {
				fprintf(fp, "%x\n", tmp_audio->name[i]);
			}
			if (tmp_audio->audio_data) {
				fprintf(fp, "%x\n%x\n%x\n", tmp_audio->audio_data->allocated, tmp_audio->audio_data->volume, tmp_audio->audio_data->alen);
				for (int i = 0; i < tmp_audio->audio_data->alen; ++i) {
					fprintf(fp, "%x\n", tmp_audio->audio_data->abuf[i]);
				}
			}
			else {
				fprintf(fp, "%x\n%x\n%x\n", 1, 0, 0);
			}
			fprintf(fp,"%x\n", tmp_audio->playing);
		}
	}

	fclose(fp);
}
transform* SERALIZE_ReadTransforms(const char* fname, unsigned int amount) {
	transform* tmp_transform = calloc(1, sizeof(transform));
	unsigned int script_attached = 0, script_resource_id = 0, script_name_size = 0, flags = 0;
	FILE* fp = fopen(fname, "r");
	fscanf(fp, "%x\n%x\n%x\n%x\n", &tmp_transform->x, &tmp_transform->y,&script_attached,&script_resource_id);
	if (script_attached) {
		tmp_transform->attached_script = LIST_At(&scripts, script_resource_id);
	}
	fscanf(fp, "%x\n", &flags);
	tmp_transform->flags = flags;
	if (flags & RENDERABLE_SQUARE) {
		renderable_square* tmp_renderablesq = calloc(1,sizeof(renderable_square));
		fscanf(fp, "%x\n%x\n", &tmp_renderablesq->w, &tmp_renderablesq->h);
		tmp_transform->renderable_shape = tmp_renderablesq;
	}
	//declaring more vairables down here so the top wont be cluttered
	unsigned int size_components;
	fscanf(fp, "%x\n", &size_components);
	tmp_transform->components = calloc(size_components, sizeof(void*));
	tmp_transform->component_order = calloc(size_components, sizeof(Uint8));
	tmp_transform->size_components = size_components;
	for (int i = 0; i < size_components; ++i) {
		unsigned int tmp_component_type = 0;
		fscanf(fp, "%x\n", &tmp_component_type);
		if (tmp_component_type == COMPONENT_AUDIO_PLAYER) {
			unsigned int size_name_audio, allocated, volume, alen;
			audio_component* tmp_audio = calloc(1, sizeof(audio_component));
			fscanf(fp, "%x\n", &size_name_audio);
			char* tmp_audio_name_buffer = calloc(1, size_name_audio);
			for (int i = 0; i < size_name_audio; ++i) {
				fscanf(fp, "%x\n", &tmp_audio_name_buffer[i]);
			}
			fscanf(fp, "%x\n%x\n%x\n", &allocated, &volume, &alen);
			if (!allocated) {
				Uint8* tmp_audio_data_buffer = calloc(1, alen);
				for (int i = 0; i < tmp_audio->audio_data->alen; ++i) {
					fscanf(fp, "%x\n", &tmp_audio_data_buffer[i]);
				}
				tmp_audio->audio_data = Mix_QuickLoad_RAW(tmp_audio_data_buffer, alen);
				tmp_audio->audio_data->volume = volume;
			}
			fscanf(fp, "%x\n", &tmp_audio->playing);
			tmp_audio->name = tmp_audio_name_buffer, tmp_audio->name_size = size_name_audio;
			tmp_transform->components[i] = tmp_audio, tmp_transform->component_order[i] = COMPONENT_AUDIO_PLAYER;
		}
		
	}
	return tmp_transform;
	fclose(fp);
}