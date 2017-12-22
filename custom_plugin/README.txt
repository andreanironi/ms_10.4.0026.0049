Usage of Custom Plugin.

Usage of credential mapper cutom plugin.

1.Put the microstrategy.credential.mapper.inc in custom_plugin folder of the microstrategy module.
2.Change the implementation of function microstrategy_map_credentials if needed.
3.Change the MicroStrategy page's or block's plug-ins settings:

Credential Mapper File Name = microstrategy.credential.mapper.inc
Credential Mapper Function Name  = microstrategy_map_credentials

Usage of url parameter mapper cutom plugin.

1.Put the microstrategy.urlparameter.mapper.inc in custom_plugin folder of the microstrategy module.
2.Change the implementation of function microstrategy_map_url_parameters if needed.
3.Change the MicroStrategy page's or block's plug-ins settings:

URL Parameter Mapper File Name  = microstrategy.urlparameter.mapper.inc
URL Parameter Mapper Function Name  = microstrategy_map_url_parameters
