#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "patch_types.h"
// i didn't want to fill the main source file with thousands of lines of patch data
// but i also didn't want to duplicate all the patch names in a header file
// so i'm including a c file sorry lads
#include "patches.c"

#define CLOSE_PATCHER \
	if (!silent) { \
		puts("Press Enter to close the patcher."); \
		getchar(); \
	} \
	exit(1);
#define puts if (!silent) puts
#define printf if (!silent) printf
#define wait() if (!silent) while (getchar() != '\n');

Patch patches[] = {
	{.bytes = upx_80, .name = "UPX unpacked header adjustment", .type = UPX},
	{.bytes = mempatch, .name = "Memory patch", .type = MEM},

	{.bytes = joypatch_80, .name = "GM8.0 joystick patch", .type = JOY},
	{.bytes = schedpatch_80, .name = "GM8.0 scheduler patch", .type = SCHED},
	{.bytes = schedpatch_80upx, .name = "GM8.0 (UPX unpacked) scheduler patch", .type = SCHED},
	{.bytes = inputlagpatch_80, .name = "GM8.0 input lag patch", .type = INPUTLAG},
	{.bytes = dplaypatch_80, .name = "GM8.0 DirectPlay patch", .type = DPLAY},

	{.bytes = joypatch_81_65, .name = "GM8.1.65 joystick patch", .type = JOY},
	{.bytes = schedpatch_81_65, .name = "GM8.1.65 scheduler patch", .type = SCHED},
	{.bytes = inputlagpatch_81_65, .name = "GM8.1.65 input lag patch", .type = INPUTLAG},
	{.bytes = dplaypatch_81_65, .name = "GM8.1.65 DirectPlay patch", .type = DPLAY},

	{.bytes = joypatch_81_71, .name = "GM8.1.71 joystick patch", .type = JOY},
	{.bytes = schedpatch_81_71, .name = "GM8.1.71 scheduler patch", .type = SCHED},
	{.bytes = inputlagpatch_81_71, .name = "GM8.1.71 input lag patch", .type = INPUTLAG},
	{.bytes = dplaypatch_81_71, .name = "GM8.1.71 DirectPlay patch", .type = DPLAY},

	{.bytes = joypatch_81_135, .name = "GM8.1.135 joystick patch", .type = JOY},
	{.bytes = schedpatch_81_135, .name = "GM8.1.135 scheduler patch", .type = SCHED},
	{.bytes = inputlagpatch_81_135, .name = "GM8.1.135 input lag patch", .type = INPUTLAG},
	{.bytes = dplaypatch_81_135, .name = "GM8.1.135 DirectPlay patch", .type = DPLAY},

	{.bytes = joypatch_81_140, .name = "GM8.1.140 joystick patch", .type = JOY},
	{.bytes = schedpatch_81_140, .name = "GM8.1.140 scheduler patch", .type = SCHED},
	{.bytes = inputlagpatch_81_140, .name = "GM8.1.140 input lag patch", .type = INPUTLAG},
	{.bytes = dplaypatch_81_140, .name = "GM8.1.140 DirectPlay patch", .type = DPLAY},

	{.bytes = joypatch_81_141, .name = "GM8.1.141 joystick patch", .type = JOY},
	{.bytes = schedpatch_81_141, .name = "GM8.1.141 scheduler patch", .type = SCHED},
	{.bytes = inputlagpatch_81_141, .name = "GM8.1.141 input lag patch", .type = INPUTLAG},
	{.bytes = resetpatch_81_141, .name = "GM8.1.141 display reset patch", .type = RESET},
	{.bytes = dplaypatch_81_141, .name = "GM8.1.141 DirectPlay patch", .type = DPLAY},
	
	{.bytes = NULL},
};

bool silent = false;

static PatchState can_patch(FILE *f, PatchByte patches[]) {
	// returns 2 if already patched, 1 if unpatched, 0 if it's not the right file
	bool unpatched = true;
	bool patched = true;
	int c;
	for (PatchByte *p = patches; p->pos != -1; p++) {
		fseek(f, p->pos, SEEK_SET);
		c = fgetc(f);
		if (c != p->orig_byte) {
			unpatched = false;
		}
		if (c != p->new_byte) {
			patched = false;
		}
	}
	if (patched){
		return DONE;
	}
	if (unpatched) {
		return ABLE;
	}
	return UNFOUND;
}

static void strcatfn(char *s, const char *fn) {
#ifdef _WIN32
	// windows files can't have quotes so no sanitation necessary
	strcat(s, "\"");
	strcat(s, fn);
	strcat(s, "\"");
#else
	// unix files can have quotes so gotta sanitize
	for (int i = 0; i < strlen(fn); i++) {
		if (fn[i] != '"') {
			*s++ = fn[i];
		} else {
			*s++ = '\\';
			*s++ = '"';
		}
	}
	*s++ = '"';
	*s++ = 0;
#endif
}

static bool prompt(const char *text) {
	if (silent) return true;
	int c = 0;
	while (c != 'y' && c != 'n' && c != 'Y' && c != 'N') {
		printf(text);
		c = getchar();
		while (getchar() != '\n');
	}
	return (c == 'y' || c == 'Y');
}

// return true if a backup can be made
static bool rename_for_backup(const char *fn, const char *bak_fn) {
	puts("Making backup...");
	bool can_backup = true;
	// rename the original
	if (rename(fn, bak_fn)) {
		if (errno == EEXIST) {
			errno = 0;
			printf("There is already a file in location %s\n", bak_fn);
			if (prompt("Overwrite it? [y/n] ")) {
				if (remove(bak_fn)) {
					printf("Could not remove existing file (errno %i).\n", errno);
					if (prompt("Continue without making a backup? [y/n] ")) {
						can_backup = false;
					} else {
						CLOSE_PATCHER;
					}
				} else {
					if (rename(fn, bak_fn)) {
						printf("The existing file was deleted, but the backup could still not be made (errno %i).\n", errno);
						puts("I hope there wasn't anything important in there...");
						if (prompt("Continue without making a backup? [y/n] ")) {
							can_backup = false;
						} else {
							CLOSE_PATCHER;
						}
					}
				}
			} else {
				can_backup = false;
			}
		} else {
			printf("Failed to add .bak to the filename (errno %i).\n", errno);
			if (prompt("Continue without making a backup? [y/n] ")) {
				can_backup = false;
			} else {
				CLOSE_PATCHER;
			}
		}
	}
	return can_backup;
}

// de-upx if necessary, return true if unpacked
static bool upx(FILE **fp, const char *fn, const char *argv0, bool make_backup) {
	FILE *f = *fp;
	// identify UPX0 header
	fseek(f, 0x3c, SEEK_SET);
	int pe_pointer;
	fread(&pe_pointer, 4, 1, f);
	fseek(f, pe_pointer + 0x14, SEEK_SET);
	short opt_len;
	fread(&opt_len, 2, 1, f);
	fseek(f, opt_len + 2, SEEK_CUR);
	const char head1[] = "UPX0\0\0\0"; // 4th null is implicit
	for (int i = 0; i < 8; i++) {
		if (fgetc(f) != head1[i]) return false;
	}
	char *bak_fn = NULL;
	bool can_backup = false;
	if (make_backup) {
		// make backup filename
		bak_fn = malloc(strlen(fn) + 5);
		strcpy(bak_fn, fn);
		strcat(bak_fn, ".bak");
		// ask for confirmation
		printf("Will make a backup to %s\n", bak_fn);
		puts("Looks like your game was packed with UPX. We need to unpack it first.");
		puts("Please download the latest release of UPX from https://github.com/upx/upx/releases/latest");
		puts("and put upx.exe in the same directory as gm8x_fix.exe, then press Enter to unpack.");
		wait();
		// rename original
		fclose(f);
		can_backup = rename_for_backup(fn, bak_fn);
	} else {
		puts("Looks like your game was packed with UPX. We need to unpack it first.");
		puts("Please download the latest release of UPX from https://github.com/upx/upx/releases/latest");
		puts("and put upx.exe in the same directory as gm8x_fix.exe, then press Enter to unpack.");
		puts("NOTE: Making a backup is HIGHLY RECOMMENDED for UPX games!");
		wait();
		fclose(f);
	}
	// get upx path
	char *cmd_buf = calloc((strlen(fn) + 5) * 4 + strlen(argv0), 1);
	// wrap entire thing in quotes on windows because system() on windows is `funky`
#ifdef _WIN32
	cmd_buf[0] = '"';
#endif
	int path_len = strlen(argv0);
	while (path_len > 0 && argv0[path_len-1] != '/'
#ifdef _WIN32
		&& argv0[path_len-1] != '\\'
#endif
	) {
		path_len--;
	}
	if (path_len > 0) {
		strcatfn(cmd_buf, argv0);
	}
	cmd_buf[path_len+2] = 0;
#ifndef _WIN32
	cmd_buf[path_len+1] = 0;
#endif
	if (can_backup) {
		strcat(cmd_buf, "upx\" -d -o ");
		strcatfn(cmd_buf, fn);
		strcat(cmd_buf, " ");
		strcatfn(cmd_buf, bak_fn);
	} else {
		strcat(cmd_buf, "upx -d ");
		strcatfn(cmd_buf, fn);
	}
#ifdef _WIN32
	strcat(cmd_buf, "\"");
#endif
	int res = system(cmd_buf);
	if (res != 0) {
		printf("UPX unpack failed (error code %i).", res);
		if (can_backup && rename(bak_fn, fn) != 0) {
			printf("Could not restore the original file (errno %i).\n", errno);
			puts("Your game will have had .bak added to its filename, and no ");
			puts("patches have been applied.");
		} else {
			puts("The backup has been restored and the game is unchanged.");
		}
		free(bak_fn);
		free(cmd_buf);
		CLOSE_PATCHER;
	}
	if (bak_fn != NULL) free(bak_fn);
	free(cmd_buf);
	// reopen
	*fp = f = fopen(fn, "rb");
	return true;
}

static void patch_exe(FILE *f, PatchByte patches[]) {
	for (PatchByte *p = patches; p->pos != -1; p++) {
		fseek(f, p->pos, SEEK_SET);
		fputc(p->new_byte, f);
	}
}

int main(int argc, const char *argv[]) {
	// check arguments
	const char *fn = NULL;
	bool show_help = false;
	bool valid_args = true;
	bool make_backup = true;
	bool disable_patches[TYPE_COUNT] = {0};
	if (argc == 2) {
		if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
			show_help = true;
		} else {
			fn = argv[1];
		}
	} else if (argc >= 3) {
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
				show_help = true;
				break;
			} else if (strcmp(argv[i], "-s") == 0) {
				silent = true;
			} else if (strcmp(argv[i], "-nb") == 0) {
				make_backup = false;
			} else if (strcmp(argv[i], "-nj") == 0) {
				disable_patches[JOY] = true;
			} else if (strcmp(argv[i], "-nm") == 0) {
				disable_patches[MEM] = true;
			} else if (strcmp(argv[i], "-nd") == 0) {
				disable_patches[DPLAY] = true;
			} else if (strcmp(argv[i], "-ns") == 0) {
				disable_patches[SCHED] = true;
			} else if (strcmp(argv[i], "-ni") == 0) {
				disable_patches[INPUTLAG] = true;
			} else if (strcmp(argv[i], "-nr") == 0) {
				disable_patches[RESET] = true;
			} else if (fn == NULL) {
				// yeah i don't feel like figuring out something better
				fn = argv[i];
			} else {
				valid_args = false;
			}
		}
	} else {
		valid_args = false;
	}
	// funny title
	puts("Welcome to gm8x_fix v0.5.6!");
	puts("Source code is at https://github.com/skyfloogle/gm8x_fix under MIT license.");
	puts("---------------------------------------------------------------------------");
	// did the user decide to be a funnyman and disable everything
	bool all_disabled = true;
	for (int i = 0; i < TYPE_COUNT; i++) {
		if (i != UPX && !disable_patches[i]) {
			all_disabled = false;
			break;
		}
	}
	if (all_disabled) {
		puts("you literally disabled all patches what were you expecting to happen");
		CLOSE_PATCHER;
	}
	// compain about arguments if necessary
	if (!valid_args || show_help) {
		if (!valid_args) puts("Error: Invalid arguments.");
		puts("Please drag your Game Maker game onto the patcher's executable file.");
		puts("Or if you're a commandline nerd, run with this:");
		puts(" gm8x_fix [options] FILE");
		puts("Check the readme on GitHub for information on what the patches are.");
		puts("Available options include:");
		puts(" -h   Show this help. (--help also works)");
		puts(" -s   Remove commandline output and apply any available patches.");
		puts(" -nb  Disable automatic backup (please back up manually if you do this)");
		puts(" -ni  Don't offer input lag patch.");
		puts(" -nj  Don't offer joystick patch.");
		puts(" -ns  Don't offer scheduler patch.");
		puts(" -nr  Don't offer display reset patch.");
		puts(" -nm  Don't offer memory patch.");
		puts(" -nd  Don't offer DirectPlay patch.\n");
		CLOSE_PATCHER;
	}
	printf("Inspecting file: %s\n\n", fn);
	FILE *f = fopen(fn, "rb");
	if (f == NULL) {
		printf("Could not open file (errno %i).\n", errno);
		CLOSE_PATCHER;
	}
	if (fgetc(f) != 'M' || fgetc(f) != 'Z') {
		fclose(f);
		puts("This is not an executable file.");
		CLOSE_PATCHER;
	}
	// de-upx if necessary
	bool unpacked_upx = upx(&f, fn, argv[0], make_backup);
	// identify patches
	bool any_patch_applied = false;
	bool can_apply_any = false;
	bool upx_found = false;
	for (Patch *patch = patches; patch->bytes != NULL; patch++) {
		patch->state = can_patch(f, patch->bytes);
		if (disable_patches[patch->type] && patch->state == ABLE) {
			patch->state = UNFOUND;
		}
		if (patch->type != MEM) {
			if (patch->state == ABLE) {
				if (patch->type == UPX) {
					upx_found = true;
				}
				can_apply_any = true;
			} else if (patch->state == DONE) {
				any_patch_applied = true;
			}
		}
	}

	// list patches
	if (!can_apply_any && !any_patch_applied) {
		puts("This game cannot be patched. It may not be a GameMaker 8.0 or 8.1 game.");
		fclose(f);
		CLOSE_PATCHER;
	}
	if (unpacked_upx && can_apply_any && !upx_found) {
		puts("Unpacked with UPX, but header offset wasn't recognised. I haven't seen this before, please file an issue on the GitHub.");
		puts("You can continue applying patches if you want by pressing Enter.");
		wait();
	}
	if (any_patch_applied) {
		puts("Patches already applied:");
		for (Patch *patch = patches; patch->bytes != NULL; patch++) {
			if (patch->state == DONE) {
				printf("* %s\n", patch->name);
			}
		}
	}
	if (can_apply_any) {
		puts("Patches that can be applied:");
		for (Patch *patch = patches; patch->bytes != NULL; patch++) {
			if (patch->state == ABLE) {
				if (patch->type == UPX) {
					printf("* %s (required, I won't ask for confirmation)\n", patch->name);
				} else {
					printf("* %s\n", patch->name);
				}
			}
		}
	} else {
		puts("No new patches can be applied.");
		fclose(f);
		CLOSE_PATCHER;
	}
	// if we unpacked upx it's already backed up
	if (!unpacked_upx && make_backup) {
		// make backup filename
		char *bak_fn = malloc(strlen(fn) + 5);
		strcpy(bak_fn, fn);
		strcat(bak_fn, ".bak");
		// ask for confirmation
		printf("Will make a backup to %s\n", bak_fn);
		printf("Press Enter to make a backup and choose patches to apply. ");
		wait();
		fclose(f); // i waited until here to close it so you can't mess with the file before confirming
		bool can_backup = rename_for_backup(fn, bak_fn);
		if (can_backup) {
			// copy it to the original location
			char *copy_cmd = malloc(strlen(bak_fn) * 4);
#ifdef _WIN32
			strcpy(copy_cmd, "copy ");
#else
			strcpy(copy_cmd, "cp ");
#endif
			strcatfn(copy_cmd, bak_fn);
			strcat(copy_cmd, " ");
			strcatfn(copy_cmd, fn);
			int res = system(copy_cmd);
			if (res != 0) {
				printf("File copy failed (error code %i).\n", res);
				if (rename(bak_fn, fn) != 0) {
					printf("Could not restore the original file (errno %i).\n", errno);
					puts("Your game will have had .bak added to its filename, and no ");
					puts("patches have been applied.");
				} else {
					puts("The backup has been restored and the game is unchanged.");
				}
				free(bak_fn);
				free(copy_cmd);
				CLOSE_PATCHER;
			}
			free(bak_fn);
			free(copy_cmd);
		} else {
			puts("Not backing up.");
		}
	} else {
		fclose(f);
	}
	// apply the patches
	f = fopen(fn, "rb+");
	bool joy_patched = false;
	for (Patch *patch = patches; patch->bytes != NULL; patch++) {
		if (patch->type == JOY && patch->state == DONE) {
			joy_patched = true;
		}
		if (patch->state == ABLE) {
			bool able = false;
			if (patch->type == UPX) {
				able = true;
			} else {
				if (patch->type == SCHED && !joy_patched) {
					puts("It looks like the joystick patch wasn't applied. It's best to apply that if you're going to use the scheduler patch.");
				}
				printf("Apply %s? [y/n] ", patch->name);
				able = prompt("");
			}
			if (able) {
				patch_exe(f, patch->bytes);
				if (patch->type == JOY) {
					joy_patched = true;
				}
			}
		}
	}
	fclose(f);
	puts("All done!");
	puts("Press Enter to close the patcher.");
	wait();
	return 0;
}
