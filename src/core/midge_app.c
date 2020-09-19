/* midge_app.c */

#include "core_definitions.h"
#include "render/render_thread.h"
#include <time.h>

#include "core/midge_app.h"

// void *callit(void *state)
// {
//   printf("!!callit-mc %p\n", state);
//   render_thread_info *global_data->render_thread = (render_thread_info *)state;

//   global_data->render_thread->thread_info->has_concluded = 1;
//   return NULL;
// }

// int dothecall(void *something)
// {
//   void *(*rout)(void *) = (void *(*)(void *))something;
//   printf("dothecall\n");
//   rout(NULL);
//   printf("dothecall-after-rout-call\n");
//   return 0;
// }

// int begin_silly_thread()
// {
//   printf("begin_silly_thread\n");
//   dothecall(&callit);
//   // dothecall(callit); // TODO -- C implicity converts the function so its the same as the above line, this doesn't
//   // work atm in midge...

//   return 0;
// }

int begin_render_thread()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  global_data->render_thread = (render_thread_info *)malloc(sizeof(render_thread_info));
  global_data->render_thread->render_thread_initialized = false;
  {
    // Resource Queue
    pthread_mutex_init(&global_data->render_thread->resource_queue.mutex, NULL);
    global_data->render_thread->resource_queue.count = 0;
    global_data->render_thread->resource_queue.allocated = 0;

    // Render Queue
    pthread_mutex_init(&global_data->render_thread->render_queue.mutex, NULL);
    global_data->render_thread->render_queue.count = 0;
    global_data->render_thread->render_queue.allocated = 0;

    pthread_mutex_init(&global_data->render_thread->input_buffer.mutex, NULL);
    global_data->render_thread->input_buffer.event_count = 0;
  }

  // -- Start Thread
  // dothecall(&callit);
  // printf("global_data->render_thread:%p &global_data->render_thread:%p\n", global_data->render_thread,
  // global_data->render_thread);

  begin_mthread(&midge_render_thread, &global_data->render_thread->thread_info, (void *)global_data->render_thread);
}

void complete_midge_app_compile()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  const char *remainder_app_source_files[] = {
      // "src/ui/ui_definitions.h",
      // "src/control/mc_controller.h",
      "src/modules/app_modules.h",

      "src/render/render_common.c",
      "src/env/hierarchy.c",
      "src/env/util.c",
      "src/env/project_management.c",
      "src/env/global_context_menu.c",
      "src/env/global_root.c",
      "src/ui/controls/panel.c",
      "src/ui/controls/text_block.c",
      "src/ui/controls/button.c",
      "src/ui/controls/context_menu.c",
      "src/ui/ui_functionality.c",
      "src/ui/ui_render.c",
      "src/control/mc_controller.c",

      // Modules
      "src/modules/modus_operandi/modus_operandi_curator.c",
      NULL,
  };

  for (int f = 0; remainder_app_source_files[f]; ++f) {
    printf("instantiate file:'%s'\n", remainder_app_source_files[f]);
    instantiate_all_definitions_from_file(global_data->global_node, remainder_app_source_files[f], NULL);
  }
}

extern "C" {
void mcc_initialize_input_state();
void mcc_update_xcb_input();
void mui_initialize_core_ui_components();
void mui_render_element_headless(mc_node *element_node);
void mui_render_element_present(image_render_queue *render_queue, mc_node *element_node);

// Modules
void init_modus_operandi_curator();
}




void initialize_midge_components()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mcc_initialize_input_state();

  mui_initialize_ui_state(&global_data->ui_state);
  mui_initialize_core_ui_components();

  // Environment
  mca_init_global_context_menu();
  mca_init_global_node_context_menu_options();

  // Modules
  init_modus_operandi_curator();
}

void midge_initialize_app(struct timespec *app_begin_time)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);
  global_data->app_begin_time = app_begin_time;

  global_data->requires_layout_update = true;

  struct timespec source_load_complete_time;
  clock_gettime(CLOCK_REALTIME, &source_load_complete_time);

  printf("#######################\n  <<<< MIDGE >>>>\n\nCore Compile took %.2f "
         "seconds\n",
         source_load_complete_time.tv_sec - global_data->app_begin_time->tv_sec +
             1e-9 * (source_load_complete_time.tv_nsec - global_data->app_begin_time->tv_nsec));

  // Asynchronously begin the render thread containing vulkan and xcb
  begin_render_thread();

  // Compile the remainder of the application
  complete_midge_app_compile();
  printf("midge compilation complete\n");

  // Initialize main thread
  initialize_midge_components();
  printf("midge components initialized\n");

  // Wait for render thread initialization and all resources to load before
  // continuing with the next set of commands
  bool waited = false;
  while (!global_data->render_thread->render_thread_initialized || global_data->render_thread->resource_queue.count) {
    waited = true;
    usleep(1);
  }

  // Set properties

  // Completed
  struct timespec load_complete_frametime;
  clock_gettime(CLOCK_REALTIME, &load_complete_frametime);
  printf("##################################\n"
         "         <<<< MIDGE >>>>\n"
         "\n"
         "Post-Core load took %.2f seconds (*%s)\n"
         "App Begin took %.2f seconds\n"
         "##################################\n",
         load_complete_frametime.tv_sec - source_load_complete_time.tv_sec +
             1e-9 * (load_complete_frametime.tv_nsec - source_load_complete_time.tv_nsec),
         waited ? "vulkan-initialization limited" : "midge-compile-load limited",
         load_complete_frametime.tv_sec - global_data->app_begin_time->tv_sec +
             1e-9 * (load_complete_frametime.tv_nsec - global_data->app_begin_time->tv_nsec));
}

void mca_render_presentation()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  image_render_queue *sequence;
  obtain_image_render_queue(&global_data->render_thread->render_queue, &sequence);
  sequence->render_target = NODE_RENDER_TARGET_PRESENT;
  sequence->clear_color = COLOR_NEARLY_BLACK;
  // printf("global_data->screen : %u, %u\n", global_data->screen.width,
  // global_data->screen.height);
  sequence->image_width = global_data->screen.width;
  sequence->image_height = global_data->screen.height;
  sequence->data.target_image.image_uid = global_data->present_image_resource_uid;

  for (int a = 0; a < global_data->children->count; ++a) {
    switch (global_data->children->items[a]->type) {
    case NODE_TYPE_UI:
      mui_render_element_present(sequence, global_data->children->items[a]);
      break;
    case NODE_TYPE_VISUAL_PROJECT:
      mca_render_project_present(sequence, global_data->children->items[a]);
      break;
    default:
      MCerror(296, "mca_render_presentation>|Unsupported node type:%i", global_data->children->items[a]->type);
    }
  }
}

void midge_run_app()
{
  printf("~midge_run_app()~\n");

  global_root_data *global_data;
  obtain_midge_global_root(&global_data);
  // printf("defaultfont-0:%u\n", global_data->default_font_resource);
  // printf("global_data->ui_state:%p\n", global_data->ui_state);

  struct timespec prev_frametime, current_frametime, logic_update_frametime;
  clock_gettime(CLOCK_REALTIME, &current_frametime);
  clock_gettime(CLOCK_REALTIME, &logic_update_frametime);

  mc_input_event *input_event = (mc_input_event *)malloc(sizeof(mc_input_event));
  input_event->type = INPUT_EVENT_NONE;
  input_event->altDown = false;
  input_event->ctrlDown = false;
  input_event->shiftDown = false;
  input_event->handled = true;

  int DEBUG_secs_of_last_5sec_update = 0;

  frame_time *elapsed = (frame_time *)calloc(sizeof(frame_time), 1);
  int ui = 0;
  while (1) {
    if (global_data->render_thread->thread_info->has_concluded) {
      printf("Render Thread Aborted abnormally.\nShutting down...\n");
      break;
    }

    // // Time
    bool logic_update_due = false;
    {
      // TODO DEBUG
      usleep(10);

      long ms;  // Milliseconds
      time_t s; // Seconds
      memcpy(&prev_frametime, &current_frametime, sizeof(struct timespec));
      clock_gettime(CLOCK_REALTIME, &current_frametime);

      elapsed->frame_secs = current_frametime.tv_sec - prev_frametime.tv_sec;
      elapsed->frame_nsecs = current_frametime.tv_nsec - prev_frametime.tv_nsec;
      if (elapsed->frame_nsecs < 0) {
        --elapsed->frame_secs;
        elapsed->frame_nsecs += 1e9;
      }
      elapsed->app_secs = current_frametime.tv_sec - global_data->app_begin_time->tv_sec;
      elapsed->app_nsecs = current_frametime.tv_nsec - global_data->app_begin_time->tv_nsec;
      if (elapsed->app_nsecs < 0) {
        --elapsed->app_secs;
        elapsed->app_nsecs += 1e9;
      }

      // Logic Update
      const int ONE_MS_IN_NS = 1000000;
      const int FIFTY_PER_SEC = 20 * ONE_MS_IN_NS;
      if (1e9 * (current_frametime.tv_sec - logic_update_frametime.tv_sec) + current_frametime.tv_nsec -
              logic_update_frametime.tv_nsec >
          FIFTY_PER_SEC) {
        logic_update_due = true;

        logic_update_frametime.tv_nsec += FIFTY_PER_SEC;
        if (logic_update_frametime.tv_nsec > 1000 * ONE_MS_IN_NS) {
          logic_update_frametime.tv_nsec -= 1000 * ONE_MS_IN_NS;
          ++logic_update_frametime.tv_sec;
        }
      }

      // Update Timers
      bool exit_gracefully = false;
      // for (int i = 0; i < !exit_gracefully &&
      // global_data->update_timers.count; ++i) {
      //   update_callback_timer *timer =
      //   global_data->update_timers.callbacks[i];

      //   if (!timer->update_delegate || !(*timer->update_delegate)) {
      //     continue;
      //   }

      //   // if (logic_update_due) {
      //   //   printf("%p::%ld<>%ld\n", timer->update_delegate,
      //   timer->next_update.tv_sec, current_frametime.tv_sec);
      //   // }
      //   if (current_frametime.tv_sec > timer->next_update.tv_sec ||
      //       (current_frametime.tv_sec == timer->next_update.tv_sec &&
      //        current_frametime.tv_nsec >= timer->next_update.tv_nsec)) {
      //     // Update
      //     {
      //       void *vargs[2];
      //       vargs[0] = (void *)&elapsed;
      //       vargs[1] = (void *)&timer->state;
      //       int mc_res = (*timer->update_delegate)(2, vargs);
      //       if (mc_res) {
      //         printf("--timer->update_delegate(2, vargs):%i\n", mc_res);
      //         printf("Ending execution...\n");
      //         exit_gracefully = true;
      //         break;
      //       }
      //     }

      //     if (timer->reset_timer_on_update)
      //       increment_time_spec(&current_frametime, &timer->period,
      //       &timer->next_update);
      //     else
      //       increment_time_spec(&timer->next_update, &timer->period,
      //       &timer->next_update);
      //   }
      // }
      if (exit_gracefully) {
        break;
      }

      // Special update
      // if (current_frametime.tv_sec - DEBUG_secs_of_last_5sec_update > 4) {
      //   DEBUG_secs_of_last_5sec_update = current_frametime.tv_sec;
      //   if (special_update) {
      //     void *vargs[1];
      //     vargs[0] = &elapsed;
      //     MCcall(special_update(1, vargs));
      //   }
      // }
    }

    // Handle Input
    pthread_mutex_lock(&global_data->render_thread->input_buffer.mutex);

    if (global_data->input_state_requires_update || global_data->render_thread->input_buffer.event_count > 0)
      mcc_update_xcb_input();

    // printf("~main_input\n");
    pthread_mutex_unlock(&global_data->render_thread->input_buffer.mutex);

    if (global_data->exit_requested)
      break;
    if (global_data->render_thread->thread_info->has_concluded) {
      printf("RENDER-THREAD closed unexpectedly! Shutting down...\n");
      break;
    }

    // Update State
    // TODO ?? -- update registered methods...

    // Update Visible Layout
    {
      // As is global node update despite any requirement
      // mca_update_node_list_logic(global_data->children);
      if (global_data->requires_layout_update) {
        for (int a = 0; a < global_data->children->count; ++a) {
          printf("macl:%i\n", a);
          mca_update_node_layout_extents(global_data->children->items[a], LAYOUT_RESTRAINT_NONE);
        }

        mc_rectf bounds = {0.f, 0.f, (float)global_data->screen.width, (float)global_data->screen.height};
        for (int a = 0; a < global_data->children->count; ++a) {
          mca_update_node_layout(global_data->children->items[a], &bounds);
        }
        global_data->requires_layout_update = false;
        printf("layout updated\n");
      }

      // TODO Get rid of this field maybe?
      // global_data->ui_state->requires_update = false;
    }

    if (global_data->exit_requested)
      break;

    // Render State Changes
    if (global_data->requires_rerender) {
      pthread_mutex_lock(&global_data->render_thread->render_queue.mutex);

      // Clear the render queue
      global_data->render_thread->render_queue.count = 0;

      // Rerender all headless images
      printf("headless\n");
      mca_render_node_list_headless(global_data->children);

      // Queue the updated render
      printf("present\n");
      mca_render_presentation();

      // Release lock and allow rendering
      pthread_mutex_unlock(&global_data->render_thread->render_queue.mutex);

      // Reset States
      global_data->requires_rerender = false;
    }
  }
}

void midge_cleanup_app()
{
  printf("~midge_cleanup_app()~\n");

  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // End render thread
  end_mthread(global_data->render_thread->thread_info);

  // Destroy render thread resources
  pthread_mutex_destroy(&global_data->render_thread->resource_queue.mutex);
  pthread_mutex_destroy(&global_data->render_thread->render_queue.mutex);
  pthread_mutex_destroy(&global_data->render_thread->input_buffer.mutex);

  free(global_data->render_thread);
}