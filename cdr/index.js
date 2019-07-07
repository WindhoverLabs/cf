'use strict';

var path = require('path');

const CdrFlightAppsPlugin = require(path.join(global.CDR_INSTALL_DIR, '/commander/classes/CdrFlightAppsPlugin')).CdrFlightAppsPlugin;

module.exports = class CfeCdrFlightAppsPlugin extends CdrFlightAppsPlugin {
    constructor(config) {
        config.name = 'cf';
        config.webRoot = path.join( __dirname, 'web');  
        super(config);

        var content = {
            cf: {
                shortDescription: 'CFDP File Transfer',
                longDescription: 'CFDP File Transfer.',
                nodes: {
				    main: {
					    type: CdrFlightAppsPlugin.ContentType.LAYOUT,
					    shortDescription: 'Main',
					    longDescription: 'Main CF.',
					    filePath: 'main_layout.lyt'
				    },
				    cdh: {
					    type: CdrFlightAppsPlugin.ContentType.PANEL,
					    shortDescription: 'Command and Data Handling',
					    longDescription: 'Command counters.',
					    filePath: 'cdh.pug'
				    },
				    app: {
					    type: CdrFlightAppsPlugin.ContentType.PANEL,
					    shortDescription: 'Application',
					    longDescription: 'Application.',
					    filePath: 'app.pug'
				    },
				    down0: {
					    type: CdrFlightAppsPlugin.ContentType.PANEL,
					    shortDescription: 'Downlink 0',
					    longDescription: 'Downlink channel 0',
					    filePath: 'down0.pug'
					},
				    down1: {
					    type: CdrFlightAppsPlugin.ContentType.PANEL,
					    shortDescription: 'Downlink 1',
					    longDescription: 'Downlink channel 1',
					    filePath: 'down1.pug'
					},
				    cond: {
					    type: CdrFlightAppsPlugin.ContentType.PANEL,
					    shortDescription: 'Fault Condition',
					    longDescription: 'Fault Condition',
					    filePath: 'cond.pug'
				    },
				    config: {
					    type: CdrFlightAppsPlugin.ContentType.PANEL,
					    shortDescription: 'Configuration',
					    longDescription: 'Configuration',
					    filePath: 'config.pug'
				    },
				    engine: {
					    type: CdrFlightAppsPlugin.ContentType.PANEL,
					    shortDescription: 'Engine',
					    longDescription: 'Engine',
					    filePath: 'engine.pug'
				    },
				    memory: {
					    type: CdrFlightAppsPlugin.ContentType.PANEL,
					    shortDescription: 'Memory',
					    longDescription: 'Memory.',
					    filePath: 'memory.pug'
				    },
				    trans_diag: {
					    type: CdrFlightAppsPlugin.ContentType.PANEL,
					    shortDescription: 'Transaction Diagnostics',
					    longDescription: 'Transaction Diagnostics',
					    filePath: 'trans_diag.pug'
				    },
				    up: {
					    type: CdrFlightAppsPlugin.ContentType.PANEL,
					    shortDescription: 'Uplink',
					    longDescription: 'Uplink',
					    filePath: 'up.pug'
				    }
                }
            }
        };

    	this.addContent(content);
    }
};
