#include "graphics.h"

const char* string_sched[] = {"SCHEDULER = OTHER", "SCHEDULER = FIFO", "SCHEDULER = RR"};
const char* string_prior[] = {"PRIORITY  = ALL EQUAL", "PRIORITY  = ALL DIFF.", "PRIORITY  = CUSTOM"};
const char* string_part[] = {" -PARTITIONING = PARTITIONED- ", "-- PARTITIONING = GLOBAL --"};
const char* string_prot[] = {" PROTOCOL = PRIO_INHERITANCE", " PROTOCOL = PRIO_CEILING", " PROTOCOL = NO_PROTOCOL"};
const char* string_act_mode[] = {" ACTIV. MODE = NOW", " ACTIV. MODE = DEFERRED (with time offset)", " ACTIV. MODE = DEFERRED (no time offset)"};

#define MAX_STR_PROT strlen(string_prot[0])
#define MAX_STR_PART strlen(string_part[0])

/* s = scheduler, p = task's priority , tipo_test={0:SCHED,1:PART,2:PROT} */
void draw_system_info (int s, int part, int prot, int act_mode, int p, int tipo_test, bool verbose) {

	int 	len_stringProt_Part = MAX_STR_PROT + MAX_STR_PART + 1;
	int 	x, nc;
	int 	col_prot ,col_sched, col_part, col_act_flag; //colors of various modes
	int 	col_selected = 14; 							//selected color = YELLOW
	col_prot = col_sched = col_part = FGC;

	clear_to_color(screen, BGC);

	/*rectangle for animation*/
	rect(screen, XMIN-L-1, BASE-1, XMAX+L+1, TOP+L+1, FGC);

	/*rectangle for tasks informations*/
	rect(screen, XMIN-L-1, BASE1-1, XMAX+L+1, TOP1+L+10+1, FGC);

	if(tipo_test == SCHED)
			col_sched = col_selected;
	else if (tipo_test == PART)
			col_part = col_selected;
	else if (tipo_test == PROT)
			col_prot = col_selected;
	else if (tipo_test == TASK_FUN) {
			col_act_flag = col_selected;
			textprintf_ex(screen, font, 2 + 50, 25, col_act_flag, BGC, "%s", string_act_mode[act_mode]);
	}

	/*draw system information  ( protocol - partitioning)*/
	textprintf_ex(screen, font, 2, 10, col_prot, BGC, "%s", string_prot[prot]);

	textprintf_ex(screen, font, 2 + PIXEL_CHAR * MAX_STR_PROT, 10, col_part, BGC, "%s", string_part[part]);

	x = len_stringProt_Part * PIXEL_CHAR;
	textout_ex(screen, font, string_sched[s], x, 10, col_sched, 0);

	nc = ptask_getnumcores();
	textprintf_ex(screen, font, 2, 25, FGC, BGC, " NumCores = %d", nc);

	textout_ex(screen, font, string_prior[p], x, 25, FGC, 0);

	textout_ex(screen, font, "ACTIVATION  SEQUENCE = ", XMIN, BASE1+10, FGC, 0);
	if ( verbose ){
		textout_ex(screen, font, "            PRIORITY = ", XMIN, BASE1+20, FGC, 0);
		textout_ex(screen, font, "TERMINATION SEQUENCE = ", XMIN, BASE1+30, FGC, 0);
	}

	textout_ex(screen, font, "KEY [1-9] to activate tasks (mod != MOD_DEF_OFFSET)", 5, YWIN-20, 10, 0);
	textout_ex(screen, font, "ESC exit", XWIN-70, YWIN-20, 12, 0);
}

/* draw the identifier of the task activated */
void draw_activation (int numTActive, int idT, int prio, bool verbose){

	char str_prio[3];
	char str_id[3];
	int x;
	snprintf(str_id,3, "T%d", idT+1);
	snprintf(str_prio,3, "%d", prio);

	x = XMIN + ( (strlen("ACTIVATION  SEQUENCE = ") + 1) * PIXEL_CHAR ) + ( 3 * numTActive ) * PIXEL_CHAR;
	pthread_mutex_lock(&mxa);
	textout_ex(screen, font, str_id, x, BASE1 + 10, FGC, 0);
	if (verbose) textout_ex(screen, font, str_prio, x, BASE1 + 20, FGC, 0);
	pthread_mutex_unlock(&mxa);
}


/* Draw time information about task activations and their start time and offset */
/* The time offset are time intervals from 'ptask_init(...)' to activation of tasks*/

// modeACT={MOD_NOW=0, MOD_DEF_OFFSET=1, MOD_DEF_NO_OFFS = 2}
void draw_Timetask_info(int modeACT) {
	int i;
	int y_inizio_vline = BASE1 + 3 * ALT_RIGA + ALT_MEZZARIGA;

	vline(screen, X_LINE_VERT0, y_inizio_vline, YMAX1-10, FGC);
	vline(screen, X_LINE_VERT1, y_inizio_vline, YMAX1-10, FGC);
	vline(screen, X_LINE_VERT2, y_inizio_vline, YMAX1-10, FGC);
	hline(screen, XMIN , Y_ORRIZ_LINE , XMAX -5, FGC);
	if (modeACT == MOD_DEF_OFFSET) {
		textout_ex(screen, font, "Time offset(ms) inseriti = ", XMIN, BASE1 + 2 * ALT_RIGA, FGC, 0);

	}

	textout_ex(screen, font, "TIME ACTIVATION(ms)", X_LINE_VERT0 + 5, Y_ORRIZ_LINE - ALT_RIGA, FGC, 0);
	textout_ex(screen, font, "TIME START(ms)", X_LINE_VERT1 + 5, Y_ORRIZ_LINE - ALT_RIGA, FGC, 0);
	textout_ex(screen, font, "TIME OFFSET measured(ms)", X_LINE_VERT2 + 5 , Y_ORRIZ_LINE - ALT_RIGA, FGC, 0);

	for(i = 0; i < NUM_T_TEST; i++ ) {
		textprintf_ex(screen, font, XMIN, Y_TIMES + i * ALT_RIGA,FGC, BGC, "T%d:", i+1);
	}
}

void init_vett(int tipo , int* vett, int dim, int val) {
	int i;

	if(tipo == INIZIALIZED) {
		for(i = 0; i < dim; i++) {
			vett[i] = val;
		}
	}
	else if(tipo == DIFFERENT){
		for(i = 0; i < dim; i++) {
			vett[i] = val-i;
		}
	}
}

/*type= {PRIO_EQUAL = 0: task with equal priority,
 * 		 PRIO_DIFF  = 1: task with different priority,
 * 		 PRIO_CUSTOM= 2: task with customizable priority} */
void init_vettore_prio(int tipo, int* prio, int dim) {
	int prioMAX = PRIO;
	init_vett(tipo , prio, dim, prioMAX);
}


char *listboxAct_getter(int index, int *list_size) {
	static char *strings[] = { "NOW", "DEFERRED with OFFSET", "DEFERRED without OFFSET" };
	if (index < 0) {
		*list_size = 3;
		return NULL;
	}
	else
		return strings[index];
}

/* Dialog used to select the mode activation of tasks */
DIALOG taskFunction_dialog[(9 + NUM_T_TEST * 2)] =
	{
		/* (dialog proc) (x) (y) (w) (h) (fg) (bg) (key) (flags) (d1) (d2) (dp) (dp2) (dp3) */

		/* First object used for clear the screen */
		{ d_clear_proc, 0, 0, 0, 0, FGC, BGC, 0, 0, 0, 0, NULL, NULL, NULL },

		/* program title */
		{ d_ctext_proc, XWIN/2-150, 30, 300, 50, 14, BGC, 0, 0, 0, 0,"----- TASK FUNCTION TEST -----", NULL, NULL },

		{ d_box_proc, 20, YWIN/2-80, 600, 50, FGC, BGC, 0, 0, 0, 0, NULL, NULL, NULL },
		{ d_text_proc, XMIN, YWIN/2-60, 80, 20, FGC, BGC, 0, 0, 0, 0, "ACTIVATION FLAG setting", NULL, NULL },
		{ d_list_proc, XWIN/2+50, YWIN/2-70, 200, 30, FGC, 0, 0, 0, 0, 0, listboxAct_getter, NULL, NULL },

		{ d_button_proc, 80, YWIN-80, 161, 49, 10, 0, 0, D_EXIT, 0, 0, "OK", NULL, NULL },
		{ d_button_proc, XWIN-280, YWIN-80, 161, 49, 12, 0, 0, D_EXIT, 0, 0, "EXIT", NULL, NULL },

		{ d_box_proc, 20, YWIN/2 -30, 600, 50, FGC, BGC, 0, D_HIDDEN, 0, 0, NULL, NULL, NULL },

		/* Final object */
		{ NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL }
	};

/* Final object*/
DIALOG d_final = { NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL };

int control_bound_value(int val, bool* error_found, const char* name_value, int upper_bound) {
	const char* mess1 			= "Hai inserito un numero di ";
	const char* mess2 			= " troppo alto!!!";
	int 		len				= strlen(mess1) + strlen(name_value) + strlen(mess2) + 1;
	char 		mess_alert[len];

	snprintf(mess_alert, len, "%s%s%s", mess1, name_value, mess2);
	if(!(*error_found) && ((val > upper_bound) || (val < 0))) {
				alert(mess_alert, NULL, NULL, "OK", NULL, 0, 0);
				*error_found = true;
				return 0;
	}
	else return -1;
}

/* Errors checking on edit fields */
bool error_edit(char* string, const char* name_value, char* endptr, int val, int upper_bound) {
	char* error2 = "errore conversione in long(strtol)!!!\n";
	char* error3_4 = "Presenti lettere o simboli nei valori immessi!!!\n";
	bool error_found = false;
	char* tmp2, *tmp3;

	tmp2 = tmp3 = NULL;
	errno = 0;    		/* To distinguish success/failure after call */

	/* Checking errors*/
	bool cond_error1 = ((val == INT_MAX) || (val == INT_MIN));
	bool cond_error2 = (errno == ERANGE && cond_error1) || (errno != 0 && (val == 0));
	bool cond_error3 = (endptr == string);
	bool cond_error4 = (*endptr != '\0');

	if(cond_error1 || cond_error2 || cond_error3 || cond_error4) {
		error_found = true;

		if (cond_error2) {
			perror(error2);
		}

		if (cond_error2 || cond_error3 || cond_error4) {
			tmp3 = error3_4;

			alert(tmp2, tmp3, NULL, "OK", NULL, 0, 0);
		}

	}

	control_bound_value(val, &error_found, name_value, upper_bound);

	return error_found;
}


void activate_offset_box (int var){
	int i;

	for(i = 7; i <= (NUM_T_TEST * 2 + 7); i++) {

		taskFunction_dialog[i].flags 	= D_DIRTY;
		if ( var==ON ) taskFunction_dialog[i].fg = FGC;
		else if ( var==OFF ) taskFunction_dialog[i].fg = 0;
	}
}

/* It draws dinamically  the offset's field */
void update_dialogAct (char textOffset[NUM_T_TEST][W_TEXT_OFFSET+1], char editOffset[NUM_T_TEST][NUM_CIF_OFFSET+1]){

	int i, j;
	int offset = 0;
	int default_offset = 10000;
	int sum_offset = 0;

	for(i = 0; i < NUM_T_TEST; i++) {
			snprintf(textOffset[i], W_TEXT_OFFSET + 1, "Offs%d:", i + 1);
			snprintf(editOffset[i], NUM_CIF_OFFSET + 1, "%d", sum_offset);
			sum_offset += default_offset;
		}

	/* Temporary dialog */
	DIALOG d7 = taskFunction_dialog[7]; // 'hidden' box

	//general structure: proc, x, y, w, h, fg, bg, key, flags, d1, d2, dp, dp2, NULL
	DIALOG d_temp_text = {	d_text_proc, XMIN, d7.y + (d7.h)/2,
							W_TEXT_OFFSET * PIXEL_CHAR,	/*weight text in pixel 72*/
							20, FGC, BGC, 0, 0, 0, 0, "Offs0:" , NULL, NULL
						 };

	DIALOG d_temp_edit = {	d_edit_proc, XMIN + d_temp_text.w , d7.y + (d7.h)/2,
							(NUM_CIF_OFFSET + 1) * PIXEL_CHAR, //W_EDIT_PRIO * PIXEL_CHAR,
							20, FGC, BGC, 0, 0, NUM_CIF_OFFSET, 0, editOffset[0], NULL, NULL
						 };

	for(i = 0; i < NUM_T_TEST; i++) {

			j = 8 + i * 2;

			/* TEXT FIELD Priority (ex. p0:)*/
			taskFunction_dialog[j] 			= d_temp_text;
			taskFunction_dialog[j].x 		= XMIN + i * ( d_temp_text.w + d_temp_edit.w + offset) ;
			//snprintf(sched_dialog[j].dp, lun_text_prio, "P%d:", i);
			taskFunction_dialog[j].dp 		= textOffset[i];
			taskFunction_dialog[j].flags 	= D_HIDDEN; //D_DIRTY;

			/* EDIT FIELD */
			j++;
			taskFunction_dialog[j] 			= d_temp_edit;
			taskFunction_dialog[j].x 		= taskFunction_dialog[j-1].x + d_temp_text.w;
			taskFunction_dialog[j].dp  		= editOffset[i];
			taskFunction_dialog[j].flags 	= D_HIDDEN; //D_DIRTY;

	}
	/*Final object*/
	taskFunction_dialog[8 + (NUM_T_TEST) * 2] = d_final;

}

/* Function to select type of activation mode */
int select_act(ptime* v_offset, int numTask) {

	int i, id, ret, mode;
	int val;
	bool box_offset_active = false;

	char* endptr;
	char text_offset[NUM_T_TEST][W_TEXT_OFFSET+1]; //all strings OFFSET "Px" (ex.text_offset[0]="Offs0:")
	char edit_offset[NUM_T_TEST][NUM_CIF_OFFSET+1]; //all strings of edit fields

	/* add editable fields (offset) to dialog */
	update_dialogAct (text_offset, edit_offset);

	show_mouse(screen);

	DIALOG_PLAYER *player = init_dialog(taskFunction_dialog, -1);

	while (update_dialog(player)) {

			/* reading activation mod (NOW, DEF offset, DEFFERED no offset)*/
			mode = taskFunction_dialog[4].d1;

			if(mode == MOD_DEF_OFFSET && !box_offset_active) {

				box_offset_active = true;

				/* it makes offset's box visible and active */
				activate_offset_box (ON);
			}
			if( mode != MOD_DEF_OFFSET && box_offset_active ) {

				box_offset_active = false;

				/*  it makes offset's box invisible and inactive */
				activate_offset_box (OFF);

			}
			if(  mode == MOD_DEF_OFFSET) {

				/* index of object get focus */
				id = find_dialog_focus(taskFunction_dialog);

				/* ex.: if tasks are 5, the index of editable fields to checking are 12 14 16 18 20:
				 * we dont'check editable field that get focus*/

				for(i = 9; i <= (NUM_T_TEST * 2 + 7) && i != id; i = i + 2) {
						val = strtol(taskFunction_dialog[i].dp, &endptr, 10);
						if (error_edit (taskFunction_dialog[i].dp, "offset", endptr, val, OFFSET_MAX)) {
							snprintf (taskFunction_dialog[i].dp, NUM_CIF_OFFSET + 1, "%d", OFFSET_MAX);
							taskFunction_dialog[i].flags = D_DIRTY;
						}
				}

			}

	}

	ret = shutdown_dialog(player);
	scare_mouse();
	for(i = 0; i < NUM_T_TEST; i++) {
				v_offset[i] = strtol(edit_offset[i], &endptr, 10);
	}

	if (ret == 6 || ret < 0)
			return -1;
		else {
			return mode;
		}
}
