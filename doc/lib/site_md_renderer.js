module.exports = {
  normal: function (marked) {
    var renderer = new marked.Renderer();

    renderer.heading = function (text, level) {
      return '<h'+level+' id="'+text+'">'+text+'</h'+level+'>\n';
    }

    renderer.link = function (href, title, text) {
      if (href.startsWith('http')) {
        return '<a href="'+href+'" target="_blank">'+text+'</a>';
      }
      else {
        return '<a href="'+href+'">'+text+'</a>';
      }
    }

    return renderer;
  },

  help: function (marked, identifier) {
    var renderer = new marked.Renderer();

    renderer.heading = function (text, level) {
      if (1 == level) {
        return '';
      }
      level += 2;
      return '<h'+level+' id="'+identifier+'__'+text+'">'+text+'</h'+level+'>\n';
    }

    renderer.link = function (href, title, text) {
      if (href.startsWith('http')) {
        return '<a href="'+href+'" target="_blank">'+text+'</a>';
      }
      else if (href.startsWith('#')) {
        return '<a href="#'+identifier+'__'+href.substring(1)+'">'+text+'</a>';
      }
      else if (href.startsWith('__#')) {
        return '<a href="#'+href.substring(3)+'">'+text+'</a>';
      }
      else if (href.match(/.+\.md#.+/)) {
        var matches = href.match(/(.+)\.md#(.+)/);
        var anchor = '=' == matches[2] ? text : matches[2];
        return '<a href="#'+matches[1]+'__'+anchor+'">'+text+'</a>';
      }
      else if (href.match(/.+\.md/)) {
        var matches = href.match(/(.+)\.md/);
        return '<a href="#'+matches[1]+'">'+text+'</a>';
      }
      else {
        return '<a href="'+href+'">'+text+'</a>';
      }
    }

    return renderer;
  }
};
