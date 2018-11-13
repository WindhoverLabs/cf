'use strict';

var path = require('path');

const CdrPlugin = require(path.join(global.CDR_INSTALL_DIR, '/commander/classes/CdrPlugin')).CdrPlugin;

module.exports = class CfeCdrPlugin extends CdrPlugin {
  constructor(urlBase) {
    super('cf', path.join(__dirname, 'web', urlBase));
  }

  getContent() {
    var result = {
      shortDescription: 'CFDP File Transfer',
      longDescription: 'CFDP File Transfer.',
      nodes: {
				main: {
					type: CdrPlugin.ContentType.LAYOUT,
					shortDescription: 'Main',
					longDescription: 'Main CF.',
					filePath: '/main_layout.lyt'
				},
				cdh: {
					type: CdrPlugin.ContentType.PANEL,
					shortDescription: 'Command and Data Handling',
					longDescription: 'Command counters.',
					filePath: '/cdh.pug'
				},
				app: {
					type: CdrPlugin.ContentType.PANEL,
					shortDescription: 'Application',
					longDescription: 'Application.',
					filePath: '/app.pug'
				},
				down: {
					type: CdrPlugin.ContentType.PANEL,
					shortDescription: 'Downlink',
					longDescription: 'Downlink',
					filePath: '/down.pug'
				},
				cond: {
					type: CdrPlugin.ContentType.PANEL,
					shortDescription: 'Fault Condition',
					longDescription: 'Fault Condition',
					filePath: '/cond.pug'
				},
				config: {
					type: CdrPlugin.ContentType.PANEL,
					shortDescription: 'Configuration',
					longDescription: 'Configuration',
					filePath: '/config.pug'
				},
				engine: {
					type: CdrPlugin.ContentType.PANEL,
					shortDescription: 'Engine',
					longDescription: 'Engine',
					filePath: '/engine.pug'
				},
				memory: {
					type: CdrPlugin.ContentType.PANEL,
					shortDescription: 'Memory',
					longDescription: 'Memory.',
					filePath: '/memory.pug'
				},
				trans_diag: {
					type: CdrPlugin.ContentType.PANEL,
					shortDescription: 'Transaction Diagnostics',
					longDescription: 'Transaction Diagnostics',
					filePath: '/trans_diag.pug'
				},
				up: {
					type: CdrPlugin.ContentType.PANEL,
					shortDescription: 'Uplink',
					longDescription: 'Uplink',
					filePath: '/up.pug'
				}
      }
    };

    return result;
  }
};
