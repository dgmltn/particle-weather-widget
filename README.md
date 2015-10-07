Particle Weather Widget
=======================

Uses the [Particle Photon][1] plugged into a [Sparkfun Micro OLED Shield][2] as a simple weather widget.
See my [writeup on Medium][5].

[![demo](https://i.ytimg.com/vi_webp/Uq2O8_fVqQE/0.webp)](https://youtu.be/Uq2O8_fVqQE "Particle Weather Widget")

Setup
-----

Your particle project should have these 3 files (weather.ino, icons.h, oled.h),
along with the sparkfunmicrooled included library.

Adjust the time zone offset for your location in weather.ino. Flash it
to your spare Particle, and test it using curl:

```
curl -H "Authorization: Bearer asdfasdfasdfasdf" \
    https://api.spark.io/v1/devices/1234123412341234/weather \
    -d params=63.61~69.27~60.62~clear-night~1444052817~1444095007
```

Webhook
-------

Setting up the webhook to talk to forecast.io is fairly simple. Adjust the latitude
and longitude for your location in webhook.json, and go [register for an API key][3].

```
particle webhook create webhook.json
```

Here's [more documentation on Particle's webhooks][4].

Future
------

I'd like to expand this beyond weather. I tried to design the firmware to be easily
expandable. I might try to add a Gmail unread email count, or an alert if a door in
my house is left open.

Pull requests are welcome!

[1]: https://www.particle.io/
[2]: https://www.sparkfun.com/products/13628
[3]: https://developer.forecast.io/
[4]: https://docs.particle.io/guide/tools-and-features/webhooks/
[5]: https://medium.com/@dgmltn/particle-weather-widget-6f01eac5ec6c
