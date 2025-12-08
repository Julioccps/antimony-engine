#include <stdio.h>
#include "sb.h"

int main() {
	SbContext ctx = {0};
	if (sb_init(&ctx, "Antimony Engine Sandbox")) {
		printf("Failed to initialize Antimony Engine\n");
		return 1;
	}

	while (!glfwWindowShouldClose(ctx.window)) {
		glfwPollEvents();
	}

	sb_cleanup(&ctx);
	return 0;
}
