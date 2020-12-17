#ifndef PINS_H
#define PINS_H

#define RELAY_PIN 15 /**< The relay pin. The relay physically cuts the main line. */

#define TRIAC_PIN 14 /**< The triac pin. The triac controls the main line power. */
#define INTER_PIN 10 /**< The zero crossing detectorpin. It will interrupt the program each time the main line crosses 0V */
#define LED_PIN   13 /**< The built-in led. */

#define TEMP_PIN  16 /**< The temperature sensor pin. */

#define SW1_PIN 2 /**< The left switch pin. */
#define SW2_PIN 3 /**< The center switch pin. */
#define SW3_PIN 4 /**< The right switch pin. */

#endif // PINS_H