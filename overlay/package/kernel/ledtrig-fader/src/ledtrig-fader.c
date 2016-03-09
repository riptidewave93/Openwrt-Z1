/*
 * LED Kernel Fader Trigger
 *
 * Copyright 2013 Cisco Inc.
 *
 * Author: Justin Delegard <jdizzle@meraki.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/leds.h>
#include <linux/device.h>
#include <linux/rwsem.h>

struct fader {
	struct rw_semaphore	lock;
	struct delayed_work	work;
	struct led_trigger	trig;
	struct led_classdev	*shrinking;
	struct led_classdev	*growing;
	u32			speed;
};

static void fade_activate(struct led_classdev *);
static void fade_deactivate(struct led_classdev *);

static struct fader fader = {
	.trig.name     = "fade",
	.trig.activate = fade_activate,
	.trig.deactivate = fade_deactivate,
	.speed = 6,
	.growing = NULL,
	.shrinking = NULL,
};

static int led_get_brightness(struct led_classdev *led_cdev)
{
	return led_cdev->brightness;
}

/* the trigger structure is in the list, so make
 * sure we skip it when traversing. */
static struct led_classdev *next_led(struct led_classdev *lcd,
				     struct led_trigger *trig) {
	unsigned long flags;
	struct led_classdev *r;

	write_lock_irqsave(&trig->leddev_list_lock, flags);

	if (lcd->trig_list.next == &trig->led_cdevs) {
		r = list_first_entry(lcd->trig_list.next, struct led_classdev,
				     trig_list);
	} else {
		r = list_first_entry(&lcd->trig_list, struct led_classdev,
				     trig_list);
	}

	write_unlock_irqrestore(&trig->leddev_list_lock, flags);
	return r;
}

static void fader_work(struct work_struct *work)
{
	int change_shrinker = 0, change_grower = 0;
	down_read(&fader.lock);

	if (fader.shrinking) {
		if (led_get_brightness(fader.shrinking) > fader.speed) {
			led_set_brightness(fader.shrinking,
					   led_get_brightness(fader.shrinking) -
					   fader.speed);
		} else {
			led_set_brightness(fader.shrinking, 0);
			change_shrinker = 1;
		}
	}

	if (fader.growing) {
		int brightness = led_get_brightness(fader.growing) +
				 fader.speed;
		if (brightness > fader.growing->max_brightness) {
			change_grower = 1;
			led_set_brightness(fader.growing,
					   fader.growing->max_brightness);
		} else {
			led_set_brightness(fader.growing, brightness);
		}

	}

	if (change_shrinker && !fader.growing) {
		fader.growing = fader.shrinking;
		fader.shrinking = NULL;
	} else if (change_grower && !fader.shrinking) {
		fader.shrinking = fader.growing;
		fader.growing = NULL;
	} else if (change_shrinker && change_grower) {
		fader.shrinking = fader.growing;
		fader.growing = next_led(fader.shrinking, &fader.trig);

		if (fader.growing == fader.shrinking)
			fader.growing = NULL;
	}

	up_read(&fader.lock);
	schedule_delayed_work(&fader.work, HZ/30);
}

static void fade_activate(struct led_classdev *led_cdev)
{
	int needs_schedule;

	down_write(&fader.lock);
	needs_schedule = !fader.shrinking && !fader.growing;

	if (!fader.shrinking)
		fader.shrinking = led_cdev;
	else if (!fader.growing)
		fader.growing = led_cdev;
	up_write(&fader.lock);

	if (needs_schedule)
		schedule_delayed_work(&fader.work, 0);
}

static void fade_deactivate(struct led_classdev *led_cdev)
{
	struct led_classdev **kept_p = NULL, **removed_p = NULL;
	down_write(&fader.lock);

	if (fader.growing == led_cdev) {
		kept_p = &fader.shrinking;
		removed_p = &fader.growing;
		fader.growing = NULL;
	} else if (fader.shrinking == led_cdev) {
		kept_p = &fader.growing;
		removed_p = &fader.shrinking;
		fader.shrinking = NULL;
	}

	if (kept_p && removed_p && *kept_p) {
		(*removed_p) = next_led((*kept_p), &fader.trig);

		if ((*kept_p) == (*removed_p))
			(*removed_p) = NULL;
	}

	up_write(&fader.lock);

	if (!fader.growing && !fader.shrinking)
		cancel_delayed_work_sync(&fader.work);
}

static int fade_trig_init(void)
{
	INIT_DELAYED_WORK(&fader.work, fader_work);
	init_rwsem(&fader.lock);
	return led_trigger_register(&fader.trig);
}

static void fade_trig_exit(void)
{
	cancel_delayed_work_sync(&fader.work);
	led_trigger_unregister(&fader.trig);
}

module_init(fade_trig_init);
module_exit(fade_trig_exit);
MODULE_AUTHOR("Justin Delegard <jdizzle@meraki.net>");
MODULE_DESCRIPTION("Fading LED trigger");
MODULE_LICENSE("GPL");
