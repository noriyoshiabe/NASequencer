(function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']=r;i[r]=i[r]||function(){
(i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),
m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)
})(window,document,'script','https://www.google-analytics.com/analytics.js','ga');

ga('create', 'UA-77464123-X', 'auto');
ga('send', 'pageview');

document.onreadystatechange = function () {
  if ('complete' == document.readyState) {
    var appStore = document.querySelector('a#app-store');
    if (appStore) {
      appStore.addEventListener('click', function () {
        ga('send', 'event', 'AppStore', 'click');
      });
    }
  }
};
