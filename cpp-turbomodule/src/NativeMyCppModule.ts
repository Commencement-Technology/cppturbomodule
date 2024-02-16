import type { TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';
import { Int32 } from 'react-native/Libraries/Types/CodegenTypes';

interface Address {
  street: string;
  city: string;
  zipcode: string;
}

interface User {
  id: Int32;
  name: string;
  hasChildren?: boolean;
  address: Address;
}

export interface Spec extends TurboModule {
  getUsers(user: User): Array<User>;
  getUsersAsync(user: User): Promise<Array<User>>;
}

export default TurboModuleRegistry.getEnforcing<Spec>('MyCppModule');
