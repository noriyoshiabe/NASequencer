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
  }
};
