var DO = function(a) {
  this.a = a;
};

(function() {

DO.fromJSON = function(s) {
  return new DO(fromJSON(s));
};

DO.toJSON = function(a) {
  return toJSON(a, '');
};

DO.fromXML = function(s) {
  return new DO(fromXML(s));
};

DO.toXML = function(a) {
  return toXML(a);
};

DO.sanitize = function(s) {
  return sanitize(s);
}

DO.prototype = {
  render: function(target) {
    while (target.firstChild) target.removeChild(target.firstChild);
    var do_value = target.appendChild(document.createElement('div'));
    if (this.a instanceof Document) {
      renderXMLElement(do_value, this.a.documentElement);
    } else {
      renderValue(do_value, this.a);
    }
    return do_value;
  },
  toJSON: function() {
    return toJSON(this.a, '');
  },
  toXML: function() {
    return toXML(this.a.documentElement, '');
  }
};

function renderValue(do_value, a) {
  var t = typeof(a);
  if (t != 'undefined' && a == null) {
    t = 'null';
  }
  if (t == 'function' || t == 'object') {
    // function or object
//  do_value.className = 'do-node';
    var do_head = do_value.appendChild(document.createElement('div'));
    do_head.appendChild(document.createTextNode(Object.prototype.toString.apply(a)));
    var do_body = do_value.appendChild(document.createElement('div'));
    if (t == 'function') {
      // function
      renderPrintableObject(do_body, a);
    } else {
      // object
      if (a.constructor == Date) {
        // object-date
        renderPrintableObject(do_body, a);
      } else
      if (a.constructor == Array) {
        // object-array
        renderArray(do_body, a);
      } else {
        // object
        renderObject(do_body, a);
      }
    }
    initializeButton(do_head, do_body);
  } else {
    // null, undefined, string, number or boolean
    do_value.className += ' do-value-' + t;
    do_value.appendChild(document.createTextNode(a));
  }
}

function renderPrintableObject(do_body, a) {
  var do_printable_object = do_body.appendChild(document.createElement('div'));
  do_printable_object.className = 'do-value-printable-object';
  do_printable_object.appendChild(document.createTextNode(a));
}

function renderArray(do_body, a) {
  var n = a.length;
  if (n > 0) {
    var do_array = do_body.appendChild(document.createElement('table'));
    do_array.className = 'do-value-array';
    var tbody = do_array.appendChild(document.createElement('tbody'));
    for (var i = 0; i < n; i++) {
      var tr = tbody.appendChild(document.createElement('tr'));
      var do_label = tr.appendChild(document.createElement('td'));
      do_label.className = 'do-label';
      do_label.appendChild(document.createTextNode('[' + i + ']'));
      var do_value = tr.appendChild(document.createElement('td'));
      do_value.className = 'do-node';
      renderValue(do_value, a[i]);
    }
  } else {
    var do_array = do_body.appendChild(document.createElement('div'));
    do_array.className = 'do-value-array';
  }
}

function renderObject(do_body, a) {
  var n = 0;
  for (var i in a) {
    n++;
  }
  if (n > 0) {
    var do_object = do_body.appendChild(document.createElement('table'));
    do_object.className = 'do-value-object';
    var tbody = do_object.appendChild(document.createElement('tbody'));
    for (var i in a) {
      var tr = tbody.appendChild(document.createElement('tr'));
      var do_label = tr.appendChild(document.createElement('td'));
      do_label.className = 'do-label';
      do_label.appendChild(document.createTextNode(i));
      var do_value = tr.appendChild(document.createElement('td'));
      do_value.className = 'do-node';
      renderValue(do_value, a[i]);
    }
  } else {
    var do_object = do_body.appendChild(document.createElement('div'));
    do_object.className = 'do-value-object';
  }
}

function fromJSON(s) {
//if (window.JSON) {
//  return JSON.parse(s);
//}
  return eval('(' + s + ')');
}

function toJSON(a, indent) {
//if (window.JSON) {
//  return JSON.stringify(a, null, 2);
//}
  var t = typeof(a);
  if (t != 'undefined' && a == null) {
    t = 'null';
  }
  if (t == 'function' || t == 'object') {
    if (a.constructor == Array) {
      var n = a.length;
      if (indent !== undefined) {
        var b = [];
        for (var i = 0; i < n; i++) {
          b.push('\n' + indent + '  ' + toJSON(a[i], indent + '  '));
        }
        return '[' + b.join(',') + '\n' + indent + ']';
      } else {
        var b = [];
        for (var i = 0; i < a.length; i++) {
          b.push(toJSON(a[i]));
        }
        return '[' + b.join(',') + ']';
      }
    } else {
      var n = 0;
      for (var i in a) {
        n++;
      }
      if (indent !== undefined) {
        var b = [];
        for (var i in a) {
          b.push('\n' + indent + '  ' + '"' + i + '": ' + toJSON(a[i], indent + '  '));
        }
        return '{' + b.join(',') + '\n' + indent + '}';
      } else {
        var b = [];
        for (var i in a) {
          b.push('"' + i + '":' + toJSON(a[i]));
        }
        return '{' + b.join(',') + '}';
      }
    }
  } else
  if (t == 'undefined' || t == 'null') {
    return t;
  } else
  if (t == 'string') {
    return '"' + a.replace(/(\\|")/g, '\\$1').replace(/\n/g, '\\n').replace(/\r/g, '\\r').replace(/\t/g, '\\t') + '"';
  } else {
    return '' + a;
  }
}

function renderXMLElement(target, element) {
  if (element.nodeName == '#comment') {
    renderXMLText(target, element.data, '[comment]');
    return;
  }
  var do_head = target.appendChild(document.createElement('div'));
  do_head.appendChild(document.createTextNode('<' + element.nodeName + '>'));
  var do_body = target.appendChild(document.createElement('div'));
  var table = do_body.appendChild(document.createElement('table'));
  table.className = 'do-value-object';
  var tbody = table.appendChild(document.createElement('tbody'));
  var tr = tbody.appendChild(document.createElement('tr'));
  var do_label = tr.appendChild(document.createElement('td'));
  do_label.className = 'do-label';
  do_label.appendChild(document.createTextNode(element.nodeName));
  var do_value = tr.appendChild(document.createElement('td'));
  do_value.className = 'do-node';
  if (element.attributes.length > 0) {
    renderXMLAttributes(do_value, element.attributes, '[attributes]');
  }
  if (element.childNodes.length > 0) {
    var elements = [];
    var texts = [];
    for (var i = 0; i < element.childNodes.length; i++) {
      if (element.childNodes[i].nodeName == '#text') {
        var text = element.childNodes[i].data.trim().replace(/\s+/g, ' ');
        if (text != '') {
          texts.push(text);
        }
      } else {
        elements.push(element.childNodes[i]);
      }
    }
    if (elements.length > 0) {
      for (var i = 0; i < elements.length; i++) {
        renderXMLElement(do_value, elements[i]);
      }
    }
    if (texts.length > 0) {
      renderXMLText(do_value, texts.join(' '), '[text]');
    }
  }
  initializeButton(do_head, do_body);
  return do_body;
}

function renderXMLAttributes(target, attributes, label) {
  var do_head = target.appendChild(document.createElement('div'));
  do_head.appendChild(document.createTextNode(label));
  var do_body = target.appendChild(document.createElement('div'));
  var table = do_body.appendChild(document.createElement('table'));
  table.className = 'do-value-object';
  var tbody = table.appendChild(document.createElement('tbody'));
  for (var i = 0; i < attributes.length; i++) {
    var tr = tbody.appendChild(document.createElement('tr'));
    var do_label = tr.appendChild(document.createElement('td'));
    do_label.className = 'do-label';
    do_label.appendChild(document.createTextNode(attributes[i].name));
    var do_value = tr.appendChild(document.createElement('td'));
    do_value.className = 'do-node do-value-string';
    do_value.appendChild(document.createTextNode(attributes[i].value));
  }
  initializeButton(do_head, do_body);
  return do_body;
}

function renderXMLText(target, text, label) {
  var do_head = target.appendChild(document.createElement('div'));
  do_head.appendChild(document.createTextNode(label));
  var do_body = target.appendChild(document.createElement('div'));
  do_body_string = do_body.appendChild(document.createElement('div'));
  do_body_string.className = 'do-value-string';
  do_body_string.appendChild(document.createTextNode(text));
  initializeButton(do_head, do_body);
  return do_body;
}

function initializeButton(do_head, do_body) {
  (
    function(do_head, do_body) {
      do_head.className = 'do-head-opened';
      do_body.className = 'do-body-opened';
      do_head.onclick = function() {
        if (do_head.className == 'do-head-closed') {
          do_head.className = 'do-head-opened';
          do_body.className = 'do-body-opened';
        } else {
          do_head.className = 'do-head-closed';
          do_body.className = 'do-body-closed';
        }
      };
    }
  )(do_head, do_body);
}

function fromXML(s) {
    var parser = new DOMParser();
    var a = parser.parseFromString(s, 'application/xml');
    var parserErrorURI;
    try {
      parserErrorURI = parser.parseFromString('INVALID', 'application/xml').getElementsByTagName('parsererror')[0].namespaceURI;
    } catch (e) {
      parserErrorURI = null;
    }
    if (parserErrorURI !== null) {
      if (parserErrorURI === 'http://www.w3.org/1999/xhtml' && a.getElementsByTagName('parsererror').length > 0) {
        throw new SyntaxError(a.getElementsByTagName('parsererror')[0].textContent.replace('This page contains the following errors:', ''));
      }
      if (a.getElementsByTagNameNS(parserErrorURI, 'parsererror').length > 0) {
        throw new SyntaxError(a.getElementsByTagNameNS(parserErrorURI, 'parsererror')[0].textContent.replace('This page contains the following errors:', ''));
      }
    }
    return a;
}

function toXML(a, indent) {
  if (indent !== undefined) {
    if (a.nodeName == '#comment') {
      return '\n' + indent + '<--' + sanitize(a.data) + '-->';
    }
    if (a.nodeName == '#text') {
      if (a.data.trim().length == 0) {
        return '';
      }
      return '\n' + indent + sanitize(a.data.trim().replace(/\s+/g, ' '));
    }
    var s = (indent != '') ? '\n' + indent + '<' + a.nodeName : '<' + a.nodeName;
    if (a.attributes && a.attributes.length > 0) {
      for (var i = 0; i < a.attributes.length; i++) {
        s += '\n' + indent + '  ' + a.attributes[i].name + '=\"' + sanitize(a.attributes[i].value) + '\"';
      }
    }
    if (a.childNodes && a.childNodes.length > 0) {
      s += '>';
      for (var i = 0; i < a.childNodes.length; i++) {
        s += toXML(a.childNodes[i], indent + '  ');
      }
      s += '\n' + indent + '</' + a.nodeName + '>';
    } else {
      s += '/>';
    }
    return s;
  } else {
    if (a.nodeName == '#comment') {
      return '<--' + sanitize(a.data.trim().replace(/\s+/g, ' ')) + '-->';
    }
    if (a.nodeName == '#text') {
      if (a.data.trim().length == 0) {
        return '';
      }
      return sanitize(a.data.trim().replace(/\s+/g, ' '));
    }
    var s = '<' + a.nodeName;
    if (a.attributes && a.attributes.length > 0) {
      for (var i = 0; i < a.attributes.length; i++) {
        s += ' ' + a.attributes[i].name + '=\"' + sanitize(a.attributes[i].value) + '\"';
      }
    }
    if (a.childNodes && a.childNodes.length > 0) {
      s += '>';
      for (var i = 0; i < a.childNodes.length; i++) {
        s += toXML(a.childNodes[i]);
      }
      s += '</' + a.nodeName + '>';
    } else {
      s += '/>';
    }
    return s;
  }
}

function sanitize(s) {
  return s.replace(/&/g, '&amp;')
          .replace(/</g, '&lt;')
          .replace(/>/g, '&gt;')
          .replace(/'/g, '&apos;')
          .replace(/"/g, '&quot;');
}

})();
