(function(name,data){
 if(typeof onTileMapLoaded === 'undefined') {
  if(typeof TileMaps === 'undefined') TileMaps = {};
  TileMaps[name] = data;
 } else {
  onTileMapLoaded(name,data);
 }
 if(typeof module === 'object' && module && module.exports) {
  module.exports = data;
 }})("title",
{ "compressionlevel":-1,
 "height":8,
 "infinite":false,
 "layers":[
        {
         "data":[2, 3, 3, 3, 3, 4, 2, 3, 5, 3, 4, 2, 3, 3, 3, 4, 2, 3, 3, 3, 4, 1, 2, 3, 4,
            6, 7, 7, 7, 7, 8, 6, 7, 9, 7, 8, 6, 7, 7, 7, 8, 6, 7, 7, 7, 8, 1, 6, 7, 8,
            6, 10, 11, 12, 13, 14, 6, 10, 9, 10, 8, 15, 16, 10, 17, 14, 6, 10, 18, 10, 8, 1, 6, 10, 8,
            6, 19, 19, 19, 8, 1, 6, 19, 9, 19, 8, 1, 6, 19, 8, 1, 6, 19, 20, 19, 8, 1, 6, 19, 8,
            6, 19, 17, 21, 14, 1, 6, 19, 9, 19, 8, 2, 22, 19, 8, 1, 6, 19, 19, 19, 8, 1, 6, 19, 8,
            6, 23, 8, 1, 1, 1, 6, 23, 20, 23, 8, 6, 23, 23, 8, 1, 6, 23, 18, 23, 8, 1, 24, 12, 25,
            6, 26, 8, 1, 1, 1, 6, 26, 26, 26, 8, 6, 26, 26, 8, 1, 6, 26, 9, 26, 8, 1, 6, 26, 8,
            27, 21, 14, 1, 1, 1, 27, 21, 21, 21, 14, 27, 21, 21, 14, 1, 27, 21, 28, 21, 14, 1, 27, 21, 14],
         "height":8,
         "id":1,
         "name":"Tile Layer 1",
         "opacity":1,
         "type":"tilelayer",
         "visible":true,
         "width":25,
         "x":0,
         "y":0
        }],
 "nextlayerid":2,
 "nextobjectid":1,
 "orientation":"orthogonal",
 "renderorder":"right-down",
 "tiledversion":"1.11.2",
 "tileheight":8,
 "tilesets":[
        {
         "firstgid":1,
         "source":"title_tileset.tsx"
        }],
 "tilewidth":8,
 "type":"map",
 "version":"1.10",
 "width":25
});