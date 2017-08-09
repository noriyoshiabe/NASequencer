module.exports = function (marked, identifier) {
  var renderer = new marked.Renderer();

  renderer.heading = function (text, level) {
    if (4 > level) {
      return '<a name="'+text+'"></a><h'+level+'>'+text+'</h'+level+'>';
    }
    else {
      return '<h'+level+'>'+text+'</h'+level+'>';
    }
  }

  renderer.link = function (href, title, text) {
    if (href.startsWith('http')) {
      return '<a href="'+href+'" target="_blank">'+text+'</a>';
    }
    else if (href.match(/.+\.md.*/)) {
      var matches = href.match(/(.+)\.md(.*)/);
      var page = matches[1]
      var anchor = matches[2] ? '#=' == matches[2] ? '#'+text : matches[2] : '';
      return '<a href="'+page+'.html'+anchor+'">'+text+'</a>';
    }
    else {
      return '<a href="'+href+'">'+text+'</a>';
    }
  }

  return renderer;
};
