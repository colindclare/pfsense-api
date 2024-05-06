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

class APIServicesAcmeCertificateDelete extends APIModel {

    # Create our method constructor
    public function __construct() {
        parent::__construct();
        $this->packages = ["pfSense-pkg-acme"];
        $this->privileges = ["page-all", "page-service-acme"];
	$this->change_note = "Deleted ACME account key via API.";
    }

    public function validate_payload() {
        $this->__validate_account_key();
    }

    private function __validate_account_key() {
        $this->id = pfsense_pkg\acme\get_certificate_id($this->initial_data['name']);
	if (!$this->id) {
	    $this->errors[] = APIResponse\get(2293, ['name' => $this->initial_data['name']);
	}
    }

    public function action() {
        $this->del_config("installedpackages/acme/certificates/item/{$this->id}");
        $this->write_config();
	return APIResponse(0, ['msg' => "Account key {$this->initial_data['name']} successfully delete."]);
    }
}
