<?php
//   Copyright 2023 Colin Clare
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

require_once("api/framework/APIModel.inc");
require_once("api/framework/APIResponse.inc");
require_once("acme/acme.inc");
require_once("acme/acme_utils.inc");

class APIServicesAcmeAccountkeyRead extends APIModel {
    # Create our method constructor
    public function __construct() {
        parent::__construct();
        $this->packages = ["pfSense-pkg-acme"];
        $this->privileges = ["page-all", "page-service-acme"];
    }

    public function action() {
        $acme_keys = $this->get_config('installedpackages/acme/accountkeys/item');
        foreach ($acme_keys as $id => $settings) {
            $acme_keys[$id]['config_id'] = $id;
        }
        return APIResponse\get(0, $acme_keys);
    }
}
