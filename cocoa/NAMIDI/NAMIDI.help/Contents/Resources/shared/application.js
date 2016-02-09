function toggleMenu() {
  document.querySelector('body').classList.toggle('is-menu-open');
}

document.onreadystatechange = function () {
  switch (document.readyState) {
  case 'interactive':
    var topic = document.querySelector('a#topic');
    topic.addEventListener('click', toggleMenu);
    break;
  case 'complete':
    break;
  }
};

if ("HelpViewer" in window && "showTOCButton" in window.HelpViewer) {
	window.setTimeout(function () {
		window.HelpViewer.showTOCButton(true, toggleMenu, toggleMenu);
		window.HelpViewer.setTOCButton(true);
	}, 100);
}
