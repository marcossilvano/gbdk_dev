import json

try:
    a = int('aaa')
except:
    print('12345'[0:50] + 'hey')

config: dict ={
    'destination': 'dest folder',
    'auto convert': False,
    'png files': [ 
        # ListItem... 
        ]
}

class Item:
    def __init__(self, id: int = 0, name: str = None):
        self.id = id
        self.name = name

    def from_dict(self, **entries):
        self.__dict__.update(**entries)        
        return self

def to_json(obj) -> str:
    return json.dumps(obj, default=lambda obj: obj.__dict__)

items = [Item(x, 'john' + str(x)) for x in range(1,6)]
config['png files'] = items

print()

json_str: str = to_json(config)
print(json_str)

json_dict = json.loads(json_str)
print(json_dict)


# items = [Item().from_dict(x) for x in json_dict['png files']]

items = [Item(int(x['id']), x['name']) for x in json_dict['png files']]
for item in items:
    print(item.id, item.name)
