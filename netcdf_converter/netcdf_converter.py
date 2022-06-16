import netCDF4 as nc
import pandas as pd



def convert_nc_to_txt(name):
    print('---\nbeginning conversion of', name)
    # Read nc file
    ds = nc.Dataset(name+'.nc')

    # Extract variable
    variables = list(ds.variables.keys())
    print(variables)

    for variable in variables:
        var = ds.variables[variable]
        print(var.shape)
        # var = ds.variables[name]

        # Create pandas dataframe with var
        var_list=[]
        for i in var:
            var_list.append(list(i))
        df = pd.DataFrame(var_list)

        print(name+'_'+variable+'.txt')
        df.to_csv(name+'_'+variable+'.txt', header=False, index=False, sep=' ')

# name = ''
# name = ''
name = ''


# convert_nc_to_txt('color17695')
# convert_nc_to_txt('edge17695')
# convert_nc_to_txt('texture17695')

convert_nc_to_txt('tos_O1_2001-2002')
# convert_nc_to_txt('cami_0000-09-01_64x128_L26_c030918')
# convert_nc_to_txt('ECMWF_ERA-40_subset')
# convert_nc_to_txt('test_echam_spectral')