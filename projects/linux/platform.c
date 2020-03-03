// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include "platform.h"
#include "status/rot_status.h"


/**
 * Sleep for a specified number of milliseconds.
 *
 * @param msec The number of milliseconds to sleep.
 */
void platform_msleep (uint32_t msec)
{
	struct timespec sleep_time;

	sleep_time.tv_sec = msec / 1000;
	sleep_time.tv_nsec = (msec % 1000) * 1000000ULL;

	nanosleep (&sleep_time, NULL);
}


#define	PLATFORM_TIMEOUT_ERROR(code)		ROT_ERROR (ROT_MODULE_PLATFORM_TIMEOUT, code)

/**
 * Initialize a timeout based on a specific clock source.
 *
 * @param msec The number of milliseconds to use for the timeout.
 * @param clock The clock source to use.
 * @param timeout The structure that will indicated the absolute timeout.
 *
 * @return 0 if the timeout was successfully initialized or an error code.
 */
static int platform_init_timeout_from_clock (uint32_t msec, clockid_t clock,
	platform_clock *timeout)
{
	int status;

	status = clock_gettime (clock, timeout);
	if (status != 0) {
		return PLATFORM_TIMEOUT_ERROR (errno);
	}

	return platform_increase_timeout (msec, timeout);
}

/**
 * Initialize a clock structure to represent the time at which a timeout expires.
 *
 * @param msec The number of milliseconds to use for the timeout.
 * @param timeout The timeout clock to initialize.
 *
 * @return 0 if the timeout was initialized successfully or an error code.
 */
int platform_init_timeout (uint32_t msec, platform_clock *timeout)
{
	if (timeout == NULL) {
		return PLATFORM_TIMEOUT_ERROR (EINVAL);
	}

	return platform_init_timeout_from_clock (msec, CLOCK_MONOTONIC, timeout);
}

/**
 * Increase the amount of time for an existing timeout.
 *
 * @param msec The number of milliseconds to increase the timeout expiration by.
 * @param timeout The timeout clock to update.
 *
 * @return 0 if the timeout was updated successfully or an error code.
 */
int platform_increase_timeout (uint32_t msec, platform_clock *timeout)
{
	if (timeout == NULL) {
		return PLATFORM_TIMEOUT_ERROR (EINVAL);
	}

	timeout->tv_sec += (msec / 1000);
	timeout->tv_nsec += (msec % 1000) * 1000000ULL;
	if (timeout->tv_nsec > 999999999ULL) {
		timeout->tv_sec++;
		timeout->tv_nsec -= 1000000000ULL;
	}

	return 0;
}

/**
 * Initialize a clock structure to represent current tick count.
 *
 * @param currtime The platform_clock type to initialize.
 *
 * @return 0 if the current tickcount was initialized successfully or an error code.
 */
int platform_init_current_tick (platform_clock *currtime)
{
	int status;

	status = clock_gettime (CLOCK_MONOTONIC, currtime);
	if (status != 0) {
		return PLATFORM_TIMEOUT_ERROR (errno);
	}

	return 0;
}

/**
 * Determine if the specified timeout has expired.
 *
 * @param timeout The timeout to check.
 *
 * @return 1 if the timeout has expired, 0 if it has not, or an error code.
 */
int platform_has_timeout_expired (platform_clock *timeout)
{
	int status;
	struct timespec now;

	status = clock_gettime (CLOCK_MONOTONIC, &now);
	if (status != 0) {
		return PLATFORM_TIMEOUT_ERROR (errno);
	}

	if (now.tv_sec > timeout->tv_sec) {
		return 1;
	}
	else if (now.tv_sec < timeout->tv_sec) {
		return 0;
	}
	else if (now.tv_nsec < timeout->tv_nsec) {
		return 0;
	}
	else {
		return 1;
	}
}


#define	PLATFORM_MUTEX_ERROR(code)		ROT_ERROR (ROT_MODULE_PLATFORM_MUTEX, code)

/**
 * Initialize a Linux mutex.
 *
 * @param mutex The mutex to initialize.
 *
 * @return 0 if the mutex was successfully initialized or an error code.
 */
int platform_mutex_init (platform_mutex *mutex)
{
	int status = pthread_mutex_init (mutex, NULL);
	return (status == 0) ? 0 : PLATFORM_MUTEX_ERROR (status);
}

/**
 * Free a Linux mutex.
 *
 * @param mutex The mutex to free.
 *
 * @return 0 if the mutex was freed or an error code.
 */
int platform_mutex_free (platform_mutex *mutex)
{
	int status = pthread_mutex_destroy (mutex);
	return (status == 0) ? 0 : PLATFORM_MUTEX_ERROR (status);
}

/**
 * Acquire the mutex lock.
 *
 * @param mutex The mutex to lock.
 *
 * @return 0 if the mutex was successfully locked or an error code.
 */
int platform_mutex_lock (platform_mutex *mutex)
{
	int status = pthread_mutex_lock (mutex);
	return (status == 0) ? 0 : PLATFORM_MUTEX_ERROR (status);
}

/**
 * Release the mutex lock.
 *
 * @param mutex The mutex to unlock.
 *
 * @return 0 if the mutex was successfully unlocked or an error code.
 */
int platform_mutex_unlock (platform_mutex *mutex)
{
	int status = pthread_mutex_unlock (mutex);
	return (status == 0) ? 0 : PLATFORM_MUTEX_ERROR (status);
}

/**
 * Initialize a Linux recursive mutex.
 *
 * @param mutex The mutex to initialize.
 *
 * @return 0 if the mutex was successfully initialized or an error code.
 */
int platform_recursive_mutex_init (platform_mutex *mutex)
{
	pthread_mutexattr_t attr;
	int status;

	if (mutex == NULL) {
		return PLATFORM_MUTEX_ERROR (EINVAL);
	}

	status = pthread_mutexattr_init (&attr);
	if (status != 0) {
		return PLATFORM_MUTEX_ERROR (status);
	}

	status = pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
	if (status != 0) {
		return PLATFORM_MUTEX_ERROR (status);
	}

	status = pthread_mutex_init (mutex, &attr);
	return (status == 0) ? 0 : PLATFORM_MUTEX_ERROR (status);
}


#define	PLATFORM_TIMER_ERROR(code)		ROT_ERROR (ROT_MODULE_PLATFORM_TIMER, code)

/**
 * Internal thread function for running the timer.
 *
 * @param arg The timer context.
 */
static void* platform_timer_thread (void *arg)
{
	platform_timer *timer = (platform_timer*) arg;
	int status;

	pthread_mutex_lock (&timer->lock);
	while (!timer->destroy) {
		if (timer->disarm) {
			pthread_mutex_unlock (&timer->lock);
			sem_wait (&timer->timer);
			pthread_mutex_lock (&timer->lock);
		}

		if (!timer->destroy) {
			pthread_mutex_unlock (&timer->lock);

			status = sem_timedwait (&timer->timer, &timer->timeout);
			if ((status != 0) && (errno == ETIMEDOUT)) {
				pthread_mutex_lock (&timer->lock);
				if (!timer->disarm && !timer->destroy) {
					timer->disarm = 1;
					timer->callback (timer->context);
				}
			}
			else {
				pthread_mutex_lock (&timer->lock);
			}
		}
	}
	pthread_mutex_unlock (&timer->lock);

	return NULL;
}

/**
 * Create a timer that is not armed.
 *
 * @param timer The container for the created timer.
 * @param callback The function to call when the timer expires.
 * @param context The context to pass to the notification function.
 *
 * @return 0 if the timer was created or an error code.
 */
int platform_timer_create (platform_timer *timer, timer_callback callback, void *context)
{
	pthread_mutexattr_t attr;
	int status;

	if ((timer == NULL) || (callback == NULL)) {
		return PLATFORM_TIMER_ERROR (EINVAL);
	}

	memset (timer, 0, sizeof (platform_timer));

	status = sem_init (&timer->timer, 0, 0);
	if (status != 0) {
		return PLATFORM_TIMER_ERROR (errno);
	}

	pthread_mutexattr_init (&attr);
	pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
	status = pthread_mutex_init (&timer->lock, &attr);
	pthread_mutexattr_destroy (&attr);
	if (status != 0) {
		sem_destroy (&timer->timer);
		return PLATFORM_TIMER_ERROR (status);
	}

	timer->disarm = 1;
	timer->callback = callback;
	timer->context = context;

	status = pthread_create (&timer->thread, NULL, platform_timer_thread, timer);
	if (status != 0) {
		pthread_mutex_destroy (&timer->lock);
		sem_destroy (&timer->timer);
		return PLATFORM_TIMER_ERROR (status);
	}

	return 0;
}

/**
 * Start a one-shot timer.  Calling this on an already armed timer will restart the timer with the
 * specified timeout.
 *
 * @param timer The timer to start.
 * @param ms_timeout The timeout to wait for timer expiration, in milliseconds.
 *
 * @return 0 if the timer has started or an error code.
 */
int platform_timer_arm_one_shot (platform_timer *timer, uint32_t ms_timeout)
{
	int status;
	struct timespec timeout;

	if ((timer == NULL) || (ms_timeout == 0)) {
		return PLATFORM_TIMER_ERROR (EINVAL);
	}

	status = platform_init_timeout_from_clock (ms_timeout, CLOCK_REALTIME, &timeout);
	if (status != 0) {
		return status;
	}

	pthread_mutex_lock (&timer->lock);
	timer->disarm = 0;
	timer->timeout = timeout;
	status = sem_post (&timer->timer);
	pthread_mutex_unlock (&timer->lock);

	return (status == 0) ? 0 : PLATFORM_TIMER_ERROR (errno);
}

/**
 * Stop a timer.
 *
 * @param timer The timer to stop.
 *
 * @return 0 if the timer is stopped or an error code.
 */
int platform_timer_disarm (platform_timer *timer)
{
	int status = 0;

	if (timer == NULL) {
		return PLATFORM_TIMER_ERROR (EINVAL);
	}

	pthread_mutex_lock (&timer->lock);
	if (!timer->disarm) {
		timer->disarm = 1;
		status = sem_post (&timer->timer);
	}
	pthread_mutex_unlock (&timer->lock);

	return (status == 0) ? 0 : PLATFORM_TIMER_ERROR (errno);
}

/**
 * Delete and disarm a timer.  Do not delete a timer from within the context of the event callback.
 *
 * @param timer The timer to delete.
 */
void platform_timer_delete (platform_timer *timer)
{
	int status;

	if (timer != NULL) {
		pthread_mutex_lock (&timer->lock);
		timer->destroy = 1;
		timer->disarm = 1;
		status = sem_post (&timer->timer);
		pthread_mutex_unlock (&timer->lock);

		if (status == 0) {
			pthread_join (timer->thread, NULL);
			pthread_mutex_destroy (&timer->lock);
			sem_destroy (&timer->timer);
		}
	}
}
