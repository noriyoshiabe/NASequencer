module.exports = function (marked, identifier) {
  var renderer = new marked.Renderer();

  // For Help Indexer
  renderer.br = function () {
    return '<br/>';
  }

  renderer.heading = function (text, level) {
    if (1 == level) {
      return '';
    }
    level += 2;
    return '<a name="'+identifier+'__'+text+'"></a><h'+level+'>'+text+'</h'+level+'>';
  }

  renderer.link = function (href, title, text) {
    if (href.startsWith('http')) {
      return '<a href="'+href+'" target="_blank">'+text+'</a>';
    }
    else if (href.startsWith('#')) {
      return '<a class="js-transit" href="#'+identifier+'__'+href.substring(1)+'">'+text+'</a>';
    }
    else if (href.startsWith('__#')) {
      return '<a class="js-transit" href="#'+href.substring(3)+'">'+text+'</a>';
    }
    else if (href.match(/.+\.md#.+/)) {
      var matches = href.match(/(.+)\.md#(.+)/);
      var anchor = '=' == matches[2] ? text : matches[2];
      return '<a class="js-transit" href="#'+matches[1]+'__'+anchor+'">'+text+'</a>';
    }
    else if (href.match(/.+\.md/)) {
      var matches = href.match(/(.+)\.md/);
      return '<a class="js-transit" href="#'+matches[1]+'">'+text+'</a>';
    }
    else {
      return '<a href="'+href+'">'+text+'</a>';
    }
  }

  return renderer;
};
