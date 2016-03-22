document.onreadystatechange = function () {
  switch (document.readyState) {
  case 'interactive':
    break;
  case 'complete':
    var elements = document.querySelectorAll('body > div');
    for (var i = 0; i < elements.length; ++i) {
      elements[i].style.display = 'none';
    }
    new Player().start();
    break;
  }
};

Player = function Player() {
  this.body = document.querySelector('body');
};

Player.prototype.start = function () {
  this.stateChange(new Introduction(this));
};

Player.prototype.stateChange = function (state) {
  if (this.state && this.state.exit) {
    this.state.exit();
  }

  this.state = state;

  if (this.state.entry) {
    this.state.entry();
  }
};

/* Utilility */

Extend = function (base, extend) {
  for (var key in extend) {
    base[key] = extend[key];
  }
};

/* Base */

Base = {};

Base.stateChange = function (stateClass) {
  this.el.className = this.className + ' ' + stateClass;
};

Base.onTransitionEnd = function (callback) {
  var listener = function(e) {
    if (e.target == this.el) {
      callback(e);
      this.el.removeEventListener('webkitTransitionEnd', listener. false);
    }
  }.bind(this);

  this.el.addEventListener('webkitTransitionEnd', listener, false);
};

Base.show = function () {
  this.el.style.display = 'block';
};

Base.hide = function () {
  this.el.style.display = 'none';
};

/* Introduction */

Introduction = function (player) {
  this.player = player;
  this.className = 'introduction';
  this.el = document.querySelector('.' + this.className);
};

Extend(Introduction.prototype, Base);

Introduction.prototype.entry = function () {
  this.stateChange('is-stage1');
  this.show();
  setTimeout(this.onFinishStage1.bind(this), 2000);
};

Introduction.prototype.onFinishStage1 = function () {
  this.stateChange('is-stage2');
  setTimeout(this.onFinishStage2.bind(this), 2000);
};

Introduction.prototype.onFinishStage2 = function () {
  this.stateChange('is-stage3');
  setTimeout(this.onFinishStage3.bind(this), 2000);
};

Introduction.prototype.onFinishStage3 = function () {
  this.player.stateChange(new CreateNewFile(this.player));
};

Introduction.prototype.exit = function () {
  this.stateChange('is-exit');
  this.onTransitionEnd(function () {
    this.hide();
  }.bind(this));
};

/* Create New File */

CreateNewFile = function (player) {
  this.player = player;
  this.className = 'create-new-file';
  this.el = document.querySelector('.' + this.className);
  this.file = this.el.querySelector('.file');
  this.submenu = this.el.querySelector('.submenu');
  this.new = this.el.querySelector('.new');
  this.create = this.el.querySelector('.create');
};

Extend(CreateNewFile.prototype, Base);

CreateNewFile.prototype.entry = function () {
  this.show();
  setTimeout(this.entryTransition.bind(this), 0);
};

CreateNewFile.prototype.entryTransition = function () {
  this.stateChange('entry');
  setTimeout(this.showMenu.bind(this), 1000);
};

CreateNewFile.prototype.showMenu = function () {
  this.stateChange('show-menu');
  setTimeout(this.showSubmenu.bind(this), 1000);
};

CreateNewFile.prototype.showSubmenu = function () {
  this.file.classList.add('is-active');
  this.submenu.style.display = 'block';
  setTimeout(this.selectNew.bind(this), 1000);
};

CreateNewFile.prototype.selectNew = function () {
  this.new.classList.add('is-active');
  setTimeout(function () {
    this.new.classList.remove('is-active');
  }.bind(this), 1000);
  setTimeout(function () {
    this.new.classList.add('is-active');
  }.bind(this), 1050);
  setTimeout(this.showPanel.bind(this), 1500);
};

CreateNewFile.prototype.showPanel = function () {
  this.file.classList.remove('is-active');
  this.create.classList.add('is-active');
  this.stateChange('show-panel');

  setTimeout(this.clickCreate.bind(this), 1000);
}

CreateNewFile.prototype.clickCreate = function () {
  this.create.classList.remove('is-active');
  setTimeout(function () {
    this.create.classList.add('is-active');
  }.bind(this), 50);
  setTimeout(function () {
    this.player.stateChange(new Demonstration(this.player));
  }.bind(this), 150);
}

CreateNewFile.prototype.exit = function () {
  this.stateChange('is-exit');
  this.onTransitionEnd(function () {
    this.hide();
  }.bind(this));
};

/* Demonstration */

Demonstration = function (player) {
  this.player = player;
  this.className = 'demonstration';
  this.el = document.querySelector('.' + this.className);
};

Extend(Demonstration.prototype, Base);
