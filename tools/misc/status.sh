#!/bin/bash
curl -so/dev/null -D- \
    --user "$MBED_BOT" \
    --request POST \
    https://api.github.com/repos/$TRAVIS_REPO_SLUG/statuses/$TRAVIS_COMMIT \
    --data @- << DATA
{
    "state": "$1",
    "target_url": "https://example.com/build/status",
    "description": "${2-hmm}",
    "context": "travis-ci/$NAME"
}
DATA

