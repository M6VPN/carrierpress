/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_gui_sdl3.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include <SDL3/SDL.h>

#include "cp_gui.h"

#define CP_GUI_WINDOW_WIDTH	960
#define CP_GUI_WINDOW_HEIGHT	540
#define CP_GUI_MARGIN		16.0f
#define CP_GUI_LINE_HEIGHT	18.0f
#define CP_GUI_METER_WIDTH	320.0f
#define CP_GUI_METER_HEIGHT	14.0f
#define CP_GUI_METER_FULL	1.0f

static void	cp_gui_draw_bar(SDL_Renderer *, float, float, float);
static void	cp_gui_draw_panel(SDL_Renderer *, float, float, float, float,
		    const char *);
static void	cp_gui_draw_text(SDL_Renderer *, float, float, const char *);
static void	cp_gui_poll_events(struct cp_gui *);

void
cp_gui_close(struct cp_gui *gui)
{
	if (gui == NULL || !gui->active)
		return;

	if (gui->renderer != NULL)
		SDL_DestroyRenderer((SDL_Renderer *)gui->renderer);
	if (gui->window != NULL)
		SDL_DestroyWindow((SDL_Window *)gui->window);
	SDL_Quit();
	gui->active = 0;
	gui->should_stop = 0;
	gui->renderer = NULL;
	gui->window = NULL;
}

void
cp_gui_delay_ms(unsigned int delay_ms)
{
	SDL_Delay((Uint32)delay_ms);
}

int
cp_gui_init(struct cp_gui *gui)
{
	SDL_Renderer *renderer;
	SDL_Window *window;

	if (gui == NULL)
		return CP_ERR_NULL;

	memset(gui, 0, sizeof(*gui));
	if (!SDL_Init(SDL_INIT_VIDEO))
		return CP_ERR_RANGE;
	if (!SDL_CreateWindowAndRenderer("CarrierPress Monitor",
	    CP_GUI_WINDOW_WIDTH, CP_GUI_WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE,
	    &window, &renderer)) {
		SDL_Quit();
		return CP_ERR_RANGE;
	}

	gui->active = 1;
	gui->should_stop = 0;
	gui->window = window;
	gui->renderer = renderer;

	return CP_OK;
}

int
cp_gui_should_stop(const struct cp_gui *gui)
{
	if (gui == NULL)
		return 1;

	return gui->should_stop;
}

int
cp_gui_update(struct cp_gui *gui, const struct cp_gui_view *view)
{
	SDL_Renderer *renderer;
	char agc[256];
	char cat[256];
	char chain[512];
	char flags[128];
	char meters[256];
	char mode[64];
	char transport[256];

	if (gui == NULL || view == NULL || view->config == NULL ||
	    view->snapshot == NULL || !gui->active)
		return CP_ERR_NULL;

	cp_gui_poll_events(gui);
	if (gui->should_stop)
		return CP_OK;

	if (cp_gui_format_transport(view->mode, view->config, view->path,
	    view->playlist_index, view->playlist_count, view->output_device,
	    transport, sizeof(transport)) != CP_OK ||
	    cp_gui_format_mode(view->snapshot, mode, sizeof(mode)) != CP_OK ||
	    cp_gui_format_meters(view->snapshot, meters,
	    sizeof(meters)) != CP_OK ||
	    cp_gui_format_agc(view->snapshot, agc, sizeof(agc)) != CP_OK ||
	    cp_gui_format_flags(view->snapshot->stream_flags, flags,
	    sizeof(flags)) != CP_OK ||
	    cp_gui_format_chain(view->snapshot, chain, sizeof(chain)) !=
	    CP_OK ||
	    cp_gui_format_cat(view->cat_snapshot, cat, sizeof(cat)) !=
	    CP_OK)
		return CP_ERR_RANGE;

	renderer = (SDL_Renderer *)gui->renderer;
	(void)SDL_SetRenderDrawColor(renderer, 8, 12, 16, 255);
	(void)SDL_RenderClear(renderer);

	cp_gui_draw_panel(renderer, CP_GUI_MARGIN, CP_GUI_MARGIN, 928.0f,
	    72.0f, "CarrierPress Monitor");
	cp_gui_draw_text(renderer, 28.0f, 40.0f, transport);
	cp_gui_draw_text(renderer, 28.0f, 58.0f, mode);

	cp_gui_draw_panel(renderer, CP_GUI_MARGIN, 104.0f, 448.0f, 152.0f,
	    "Meters");
	cp_gui_draw_text(renderer, 28.0f, 128.0f, meters);
	cp_gui_draw_text(renderer, 28.0f, 148.0f, "Input peak");
	cp_gui_draw_bar(renderer, 140.0f, 148.0f,
	    cp_monitor_level_to_sample(view->snapshot->input_peak));
	cp_gui_draw_text(renderer, 28.0f, 170.0f, "Input RMS");
	cp_gui_draw_bar(renderer, 140.0f, 170.0f,
	    cp_monitor_level_to_sample(view->snapshot->input_rms));
	cp_gui_draw_text(renderer, 28.0f, 192.0f, "Output peak");
	cp_gui_draw_bar(renderer, 140.0f, 192.0f,
	    cp_monitor_level_to_sample(view->snapshot->output_peak));
	cp_gui_draw_text(renderer, 28.0f, 214.0f, "Output RMS");
	cp_gui_draw_bar(renderer, 140.0f, 214.0f,
	    cp_monitor_level_to_sample(view->snapshot->output_rms));

	cp_gui_draw_panel(renderer, 480.0f, 104.0f, 464.0f, 152.0f,
	    "Status");
	cp_gui_draw_text(renderer, 492.0f, 128.0f, agc);
	cp_gui_draw_text(renderer, 492.0f, 150.0f, flags);
	cp_gui_draw_text(renderer, 492.0f, 172.0f, cat);

	cp_gui_draw_panel(renderer, CP_GUI_MARGIN, 276.0f, 928.0f, 92.0f,
	    "Processing Chain");
	cp_gui_draw_text(renderer, 28.0f, 302.0f, chain);

	cp_gui_draw_panel(renderer, CP_GUI_MARGIN, 388.0f, 448.0f, 116.0f,
	    "Waveform");
	cp_gui_draw_text(renderer, 28.0f, 430.0f, "waveform reserved");

	cp_gui_draw_panel(renderer, 480.0f, 388.0f, 464.0f, 116.0f,
	    "Spectrum");
	cp_gui_draw_text(renderer, 492.0f, 430.0f, "spectrum reserved");
	cp_gui_draw_text(renderer, 28.0f, 516.0f,
	    "Keys: q or Escape stop");

	(void)SDL_RenderPresent(renderer);

	return CP_OK;
}

static void
cp_gui_draw_bar(SDL_Renderer *renderer, float x, float y, float value)
{
	SDL_FRect fill;
	SDL_FRect frame;
	float ratio;

	if (renderer == NULL)
		return;

	ratio = value / CP_GUI_METER_FULL;
	if (ratio < 0.0f)
		ratio = 0.0f;
	if (ratio > 1.0f)
		ratio = 1.0f;

	frame.x = x;
	frame.y = y;
	frame.w = CP_GUI_METER_WIDTH;
	frame.h = CP_GUI_METER_HEIGHT;
	fill = frame;
	fill.w = CP_GUI_METER_WIDTH * ratio;

	(void)SDL_SetRenderDrawColor(renderer, 42, 52, 64, 255);
	(void)SDL_RenderFillRect(renderer, &frame);
	(void)SDL_SetRenderDrawColor(renderer, 66, 176, 116, 255);
	(void)SDL_RenderFillRect(renderer, &fill);
	(void)SDL_SetRenderDrawColor(renderer, 180, 190, 196, 255);
	(void)SDL_RenderRect(renderer, &frame);
}

static void
cp_gui_draw_panel(SDL_Renderer *renderer, float x, float y, float w, float h,
	const char *title)
{
	SDL_FRect rect;

	if (renderer == NULL)
		return;

	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	(void)SDL_SetRenderDrawColor(renderer, 18, 24, 32, 255);
	(void)SDL_RenderFillRect(renderer, &rect);
	(void)SDL_SetRenderDrawColor(renderer, 120, 134, 148, 255);
	(void)SDL_RenderRect(renderer, &rect);
	if (title != NULL)
		cp_gui_draw_text(renderer, x + 12.0f, y + 8.0f, title);
}

static void
cp_gui_draw_text(SDL_Renderer *renderer, float x, float y, const char *text)
{
	if (renderer == NULL || text == NULL)
		return;

	(void)SDL_SetRenderDrawColor(renderer, 220, 226, 232, 255);
	(void)SDL_RenderDebugText(renderer, x, y, text);
}

static void
cp_gui_poll_events(struct cp_gui *gui)
{
	SDL_Event event;

	if (gui == NULL)
		return;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_EVENT_QUIT) {
			gui->should_stop = 1;
		} else if (event.type == SDL_EVENT_KEY_DOWN &&
		    (event.key.key == 'q' || event.key.key == SDLK_ESCAPE)) {
			gui->should_stop = 1;
		}
	}
}
