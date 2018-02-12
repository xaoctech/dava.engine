import sys
import json

js = json.load(sys.stdin)
if 'tags' in js and js.get('tags') is not None:
    int_tags = [int(tag) for tag in js['tags'] if tag.isdigit()]
    print max(int_tags) + 1 
else:
    print 1
