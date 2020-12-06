#include "main_h.h"

void SERALIZE_WriteTransform(const char* fname, transform* input) {
	FILE* fp = fopen("test.txt", "w");
	fprintf(fp, "%x\n%x\n", input->x, input->y);
	if (input->attached_script) {
		int test = 1;
		fprintf(fp, "%x\n%x\n",1,input->attached_script->size);
		for(int i = 0; i < input->attached_script->size; ++i)
		  fprintf(fp, "%x\n", input->attached_script->lua_data[i]);
	} 
	//print out the flags before we get into the components
	fprintf(fp, "%x\n", input->flags);
	if (input->renderable_shape) {
		if (input->flags & RENDERABLE_SQUARE) {
			renderable_square* tmp_renderablesq = input->renderable_shape;
			fprintf(fp, "%x\n%x", tmp_renderablesq->w, tmp_renderablesq->h);

		}

	}

	fclose(fp);
}
transform* SERALIZE_ReadTransforms(const char* fname, unsigned int amount) {
	transform* tmp_transform = calloc(1, sizeof(transform));
	unsigned int script_attached = 0, script_attached_size = 0, flags = 0;
	FILE* fp = fopen("test.txt", "r");
	fscanf(fp, "%x\n%x\n%x\n%x\n", &tmp_transform->x, &tmp_transform->y,&script_attached,&script_attached_size);
	if (script_attached) {
		char* tmp_lua_script_data_buffer = calloc(1, script_attached_size);
		lua_script* tmp_script_buffer = calloc(1, sizeof(lua_script));
		tmp_script_buffer->lua_fname = fname, tmp_script_buffer->lua_data = tmp_lua_script_data_buffer, tmp_script_buffer->size = script_attached_size;
		for (int i = 0; i < script_attached_size; ++i)
			fscanf(fp,"%x\n", &tmp_lua_script_data_buffer[i]);
		tmp_transform->attached_script = tmp_script_buffer;
	}
	fscanf(fp, "%x\n", &flags);
	tmp_transform->flags = flags;
	if (flags & RENDERABLE_SQUARE) {
		renderable_square* tmp_renderablesq = calloc(1,sizeof(renderable_square));
		fscanf(fp, "%x\n%x\n", &tmp_renderablesq->w, &tmp_renderablesq->h);
		tmp_transform->renderable_shape = tmp_renderablesq;
	}
	return tmp_transform;
	fclose(fp);
}