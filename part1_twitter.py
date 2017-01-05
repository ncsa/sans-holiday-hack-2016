#!/usr/bin/env python

from twitter import *
from HTMLParser import HTMLParser

import settings

twitter = Twitter(auth=OAuth(settings.oauth_token, settings.oauth_token_secret,
                             settings.consumer_key, settings.consumer_secret))

h = HTMLParser()
max_id = 0

tweets = twitter.statuses.user_timeline(screen_name="santawclaus", count=100)
while True:
    for tweet in tweets:
        print h.unescape(tweet['text'])
        max_id = tweet['id']
    if len(tweets) < 100:
        break
    tweets = twitter.statuses.user_timeline(screen_name="santawclaus", count=100, max_id=max_id)
