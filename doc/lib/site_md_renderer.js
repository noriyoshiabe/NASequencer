module.exports = {
  normal: {
    heading: function (token) {
      return '<h'+token.depth+' id="'+token.text+'">'+token.text+'</h'+token.depth+'>\n';
    },

    link: function (token) {
      if (token.href.startsWith('http')) {
        return '<a href="'+token.href+'" target="_blank" rel="noopener">'+token.text+'</a>';
      }
      else {
        return '<a href="'+token.href+'">'+token.text+'</a>';
      }
    },
  },

  help: {
    heading: function (token) {
      return '<h'+token.depth+' id="'+token.text+'">'+token.text+'</h'+token.depth+'>\n';
    },

    link: function (token) {
      if (token.href.startsWith('http')) {
        return '<a href="'+token.href+'" target="_blank" rel="noopener">'+token.text+'</a>';
      }
      else if (token.href.match(/.+\.md.*/)) {
        var matches = token.href.match(/(.+)\.md(.*)/);
        var page = matches[1];
        var anchor = matches[2] ? '#=' == matches[2] ? '#'+token.text : matches[2] : '';
        return '<a href="'+page+'.html'+anchor+'">'+token.text+'</a>';
      }
      else {
        return '<a href="'+token.href+'">'+token.text+'</a>';
      }
    },
  }
};
